#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PrototypeGASAbility.generated.h"


UCLASS()
class CUTEGIRLMOVEMENT_API UPrototypeHitAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPrototypeHitAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};