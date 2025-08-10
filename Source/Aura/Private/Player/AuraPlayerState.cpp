// Giorjorio Copyright


#include "Player/AuraPlayerState.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Net/UnrealNetwork.h"

AAuraPlayerState::AAuraPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
	
	SetNetUpdateFrequency(100.f);
}

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAuraPlayerState, Level);
	DOREPLIFETIME(AAuraPlayerState, XP);
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAuraPlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level);
}

void AAuraPlayerState::SetXP(int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void AAuraPlayerState::AddToAttributePoints(int32 InAttributePoints)
{
	// TODO: Add AttributePoints to PlayerState
}

void AAuraPlayerState::AddToSpellPoints(int32 InSpellPoints)
{
	// TODO: Add SpellPoints to PlayerState
}

void AAuraPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level);
}

void AAuraPlayerState::AddToXP(const int32 InXP)
{
	XP += InXP;
	const int32 NewLevel = LevelUpInfo->FindLevelForXp(XP);
	const int32 NumberOfLevelUps = NewLevel - Level;
	if (NumberOfLevelUps > 0)
	{
		// get rewards for current level
		for (int32 SurpassedLevel = Level; SurpassedLevel < NewLevel; ++SurpassedLevel)
		{
			const int32 AttributePointsReward = LevelUpInfo->LevelUpInformation[SurpassedLevel].AttributePointReward;
			AddToAttributePoints(AttributePointsReward);
			const int32 SpellPointsReward = LevelUpInfo->LevelUpInformation[SurpassedLevel].SpellPointReward;
			AddToSpellPoints(SpellPointsReward);
		}
		AddToLevel(NumberOfLevelUps);
		Cast<UAuraAttributeSet>(AttributeSet)->MaximizeVitalAttributes();
	}
	OnXPChangedDelegate.Broadcast(XP);
}

void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangedDelegate.Broadcast(Level);
}

void AAuraPlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChangedDelegate.Broadcast(XP);
}
