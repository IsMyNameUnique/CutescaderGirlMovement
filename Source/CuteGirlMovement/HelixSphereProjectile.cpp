#include "HelixSphereProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

AHelixSphereProjectile::AHelixSphereProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(12.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	CollisionSphere->SetGenerateOverlapEvents(false);
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	CollisionSphere->OnComponentHit.AddDynamic(this, &AHelixSphereProjectile::HandleCollisionSphereHit);

	// Artists can assign a mesh in a Blueprint child, or add more components
	// attached to CollisionSphere. This component never participates in collision.
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetGenerateOverlapEvents(false);
	VisualMesh->SetCanEverAffectNavigation(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	ProjectileMovement->Deactivate();
}

void AHelixSphereProjectile::InitializeProjectile(
	float InProjectileSpeed,
	float InProjectileLifetime,
	float InHelixRadius,
	float InHelixAngularSpeed,
	float InHelixPhase)
{
	FlightSpeed = FMath::Max(InProjectileSpeed, 0.0f);
	FlightLifetime = FMath::Max(InProjectileLifetime, 0.0f);
	HelixRadius = FMath::Max(InHelixRadius, 0.0f);
	HelixAngularSpeed = FMath::Max(InHelixAngularSpeed, 0.0f);
	HelixPhase = InHelixPhase;
	FlightOrigin = GetActorLocation();
	FlightForward = GetActorForwardVector().GetSafeNormal();
	if (FlightForward.IsNearlyZero())
	{
		FlightForward = FVector::ForwardVector;
	}
	FlightStartServerTime = GetServerTime();

	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
	}

	ApplyFlightParameters();
}

void AHelixSphereProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwnerActor = GetOwner())
	{
		CollisionSphere->IgnoreActorWhenMoving(OwnerActor, true);
	}

	ProjectileMovement->Deactivate();

	if (HasAuthority() && FlightLifetime > 0.0f)
	{
		SetLifeSpan(FlightLifetime);
	}
	ApplyFlightParameters();
}

void AHelixSphereProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeltaTime <= 0.0f || FlightSpeed <= 0.0f)
	{
		return;
	}

	UpdateFlightLocation();
}

void AHelixSphereProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHelixSphereProjectile, FlightSpeed);
	DOREPLIFETIME(AHelixSphereProjectile, FlightLifetime);
	DOREPLIFETIME(AHelixSphereProjectile, HelixRadius);
	DOREPLIFETIME(AHelixSphereProjectile, HelixAngularSpeed);
	DOREPLIFETIME(AHelixSphereProjectile, HelixPhase);
	DOREPLIFETIME(AHelixSphereProjectile, FlightOrigin);
	DOREPLIFETIME(AHelixSphereProjectile, FlightForward);
	DOREPLIFETIME(AHelixSphereProjectile, FlightStartServerTime);
}

void AHelixSphereProjectile::OnRep_FlightParameters()
{
	ApplyFlightParameters();
}

void AHelixSphereProjectile::ApplyFlightParameters()
{
	FlightForward = FlightForward.GetSafeNormal();
	if (FlightForward.IsNearlyZero())
	{
		FlightForward = FVector::ForwardVector;
	}

	HelixRight = FVector::CrossProduct(FlightForward, FVector::UpVector).GetSafeNormal();
	if (HelixRight.IsNearlyZero())
	{
		HelixRight = FVector::CrossProduct(FlightForward, FVector::RightVector).GetSafeNormal();
	}
	HelixUp = FVector::CrossProduct(HelixRight, FlightForward).GetSafeNormal();
	SetActorTickEnabled(true);
	UpdateFlightLocation();
}

void AHelixSphereProjectile::UpdateFlightLocation()
{
	FlightTime = FMath::Max(GetServerTime() - FlightStartServerTime, 0.0f);
	const float Angle = HelixPhase + HelixAngularSpeed * FlightTime;
	const FVector RadialOffset = HelixRight * FMath::Cos(Angle) + HelixUp * FMath::Sin(Angle);
	const FVector TargetLocation = FlightOrigin + FlightForward * FlightSpeed * FlightTime + RadialOffset * HelixRadius;

	FHitResult Hit;
	SetActorLocation(TargetLocation, HasAuthority(), &Hit);
}

float AHelixSphereProjectile::GetServerTime() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GameState = World->GetGameState())
		{
			return GameState->GetServerWorldTimeSeconds();
		}
		return World->GetTimeSeconds();
	}
	return 0.0f;
}

void AHelixSphereProjectile::IgnoreProjectileCollision(AActor* OtherActor)
{
	if (CollisionSphere && OtherActor)
	{
		CollisionSphere->IgnoreActorWhenMoving(OtherActor, true);
	}
}

void AHelixSphereProjectile::HandleCollisionSphereHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority() || !OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	MulticastProjectileImpact(OtherActor, Hit);
	Destroy();
}

void AHelixSphereProjectile::MulticastProjectileImpact_Implementation(AActor* HitActor, FHitResult Hit)
{
	BP_OnProjectileImpact(HitActor, Hit);
}
