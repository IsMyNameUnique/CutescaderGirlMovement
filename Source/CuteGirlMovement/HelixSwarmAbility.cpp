#include "HelixSwarmAbility.h"

#include "CuteCharacter.h"
#include "HelixSphereProjectile.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr int32 BurstPairCount = 3;
	constexpr float FallbackAimRange = 50000.0f;
	constexpr float MinimumShotInterval = 0.02f;
}

UHelixSwarmAbility::UHelixSwarmAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	ProjectileClass = AHelixSphereProjectile::StaticClass();
	static ConstructorHelpers::FClassFinder<AHelixSphereProjectile> ProjectileBlueprint(TEXT("/Game/BP_AHelixSphereProjectile"));
	if (ProjectileBlueprint.Class)
	{
		ProjectileClass = ProjectileBlueprint.Class;
	}
}

void UHelixSwarmAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACuteCharacter* Character = ActorInfo
		? Cast<ACuteCharacter>(ActorInfo->AvatarActor.Get())
		: nullptr;

	if (!Character || !Character->HasAuthority() || !Character->IsHelixSwarmHeld() || !ProjectileClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FiredProjectileCount = 0;
	bWaitingForBurstCooldown = false;

	StartBurst();
}

void UHelixSwarmAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ActorInfo->AvatarActor->GetWorldTimerManager().ClearTimer(FireTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHelixSwarmAbility::HandleFireTimer()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	ACuteCharacter* Character = ActorInfo
		? Cast<ACuteCharacter>(ActorInfo->AvatarActor.Get())
		: nullptr;

	if (!Character || !Character->HasAuthority() || !ProjectileClass)
	{
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (bWaitingForBurstCooldown)
	{
		if (!Character->IsHelixSwarmHeld())
		{
			EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, false);
			return;
		}

		bWaitingForBurstCooldown = false;
		FiredProjectileCount = 0;
		StartBurst();
		return;
	}

	// Do not check whether the button is still held here: the three-pair burst
	// completes after the player releases the button.
	SpawnProjectile(Character);

	if (FiredProjectileCount >= BurstPairCount)
	{
		Character->GetWorldTimerManager().ClearTimer(FireTimerHandle);
		bWaitingForBurstCooldown = true;
		Character->GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&UHelixSwarmAbility::HandleFireTimer,
			FMath::Max(BurstCooldown, MinimumShotInterval),
			false);
	}
}

void UHelixSwarmAbility::StartBurst()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	ACuteCharacter* Character = ActorInfo
		? Cast<ACuteCharacter>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (!Character || !Character->HasAuthority())
	{
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	SpawnProjectile(Character);
	Character->GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&UHelixSwarmAbility::HandleFireTimer,
		FMath::Max(ShotInterval, MinimumShotInterval),
		true);
}

void UHelixSwarmAbility::SpawnProjectile(ACuteCharacter* Character)
{
	if (!ProjectileClass || !Character || !Character->GetWorld())
	{
		return;
	}

	// Count a pair per call, so the ability has exactly three firing beats.
	const int32 PairIndex = FiredProjectileCount++;

	const FVector BaseSpawnOrigin = Character->GetActorLocation()
		+ FVector(0.0f, 0.0f, 50.0f)
		+ Character->GetActorForwardVector() * 60.0f;

	FVector AimPoint;
	if (Character->HasHelixAimTarget())
	{
		AimPoint = Character->GetHelixAimTarget();
	}
	else
	{
		const AController* Controller = Character->GetController();
		const FVector ViewDirection = Controller
			? Controller->GetControlRotation().Vector().GetSafeNormal()
			: Character->GetActorForwardVector().GetSafeNormal();
		AimPoint = BaseSpawnOrigin + ViewDirection * FallbackAimRange;
	}

	FVector Forward = (AimPoint - BaseSpawnOrigin).GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		const AController* Controller = Character->GetController();
		Forward = Controller
			? Controller->GetControlRotation().Vector().GetSafeNormal()
			: Character->GetActorForwardVector().GetSafeNormal();
	}
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}

	// Stagger each paired helix around the aim axis; the two strands are opposite.
	const float PairPhase = FMath::DegreesToRadians(static_cast<float>(PairIndex) * 120.0f);
	const FTransform SpawnTransform(Forward.Rotation(), BaseSpawnOrigin);
	AHelixSphereProjectile* FirstProjectile = nullptr;

	for (int32 StrandIndex = 0; StrandIndex < 2; ++StrandIndex)
	{
		const float Angle = PairPhase + static_cast<float>(StrandIndex) * PI;

		AHelixSphereProjectile* Projectile = Character->GetWorld()->SpawnActorDeferred<AHelixSphereProjectile>(
			ProjectileClass,
			SpawnTransform,
			Character,
			Character,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Projectile)
		{
			continue;
		}

		Projectile->InitializeProjectile(ProjectileSpeed, ProjectileLifetime, HelixRadius, HelixAngularSpeed, Angle);
		if (FirstProjectile)
		{
			// Keep the two projectiles in a pair from colliding with each other.
			Projectile->IgnoreProjectileCollision(FirstProjectile);
			FirstProjectile->IgnoreProjectileCollision(Projectile);
		}
		else
		{
			FirstProjectile = Projectile;
		}

		Projectile->FinishSpawning(SpawnTransform);
	}
}
