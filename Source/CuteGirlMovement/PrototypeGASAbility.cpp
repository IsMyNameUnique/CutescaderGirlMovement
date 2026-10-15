#include "PrototypeGASAbility.h"

#include "AbilitySystemComponent.h"
#include "CuteCharacter.h"

UPrototypeHitAbility::UPrototypeHitAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UPrototypeHitAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACuteCharacter* Character = ActorInfo
		? Cast<ACuteCharacter>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (!Character || !Character->HasAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Character->ActivatePrototypeHitbox();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}