// Giorjorio Copyright


#include "AbilitySystem/Abilities/AuraPassiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

void UAuraPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		AuraASC->DeactivatePassiveAbility.AddUObject(this, &UAuraPassiveAbility::ReceiveDeactivate);
	}
	
	
}

void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	if (GetAssetTags().HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

FString UAuraPassiveAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT(
		"<Title>%s</>\n\n"
		"<Small>Level: </><Level>%d</>\n\n"
		"<Default>%s</>\n\n"
		"<Small>WORK IN PROGRESS - VISUAL ONLY</>\n"
		"<Small>Gameplay effect is not implemented yet.</>"),
		*PassiveDisplayName,
		Level,
		*PassiveDescription);
}

FString UAuraPassiveAbility::GetNextLevelDescription(int32 NextLevel)
{
	return TEXT(
		"<Default>Passive upgrades are under development. "
		"Leveling this ability currently provides no gameplay bonus.</>");
}
