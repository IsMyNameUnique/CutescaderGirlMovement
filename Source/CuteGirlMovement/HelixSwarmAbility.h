#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HelixSwarmAbility.generated.h"

class AHelixSphereProjectile;

UCLASS()
class CUTEGIRLMOVEMENT_API UHelixSwarmAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHelixSwarmAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Helix Swarm|Projectile")
	TSubclassOf<AHelixSphereProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Helix Swarm", meta=(ClampMin="0.02"))
	float ShotInterval = 0.10f;

	UPROPERTY(EditDefaultsOnly, Category="Helix Swarm", meta=(ClampMin="0.0"))
	float BurstCooldown = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category="Helix Swarm", meta=(ClampMin="1.0"))
	float ProjectileSpeed = 1800.0f;

	UPROPERTY(EditDefaultsOnly, Category="Helix Swarm", meta=(ClampMin="0.0"))
	float HelixRadius = 24.0f;

	UPROPERTY(EditDefaultsOnly, Category="Helix Swarm", meta=(ClampMin="0.1"))
	float HelixAngularSpeed = 18.0f;

	UPROPERTY(EditDefaultsOnly, Category="Helix Swarm", meta=(ClampMin="0.1"))
	float ProjectileLifetime = 10.0f;

private:
	// A volley is three paired shots; held input starts another after cooldown.
	void HandleFireTimer();
	void StartBurst();
	void SpawnProjectile(class ACuteCharacter* Character);
	FTimerHandle FireTimerHandle;
	int32 FiredProjectileCount = 0;
	bool bWaitingForBurstCooldown = false;
};