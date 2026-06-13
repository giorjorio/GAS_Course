// Giorjorio Copyright

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 *
 * Singleton containing native Gameplay Tags
 */
struct FAuraGameplayTags
{
public:
	static const FAuraGameplayTags& Get() {return GameplayTags;}
 	static void InitializeNativeGameplayTags();

	/* Primary Attributes Tags */
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;

	/* Secondary Attributes Tags */
 	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;

	/* Meta Attributes Tags */
	FGameplayTag Attributes_Meta_IncomingXP;
	
	/* Damage Type Tags */
	FGameplayTag Damage;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Arcane;
	FGameplayTag Damage_Physical;
	
	/* Resistance Attributes Tags */
	FGameplayTag Attributes_Resistance;
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Arcane;
	FGameplayTag Attributes_Resistance_Physical;
	
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	
	/* Debuffs */
	FGameplayTag Debuff;
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Arcane;
	FGameplayTag Debuff_Physical;
	
	FGameplayTag Debuff_Chance;
	FGameplayTag Debuff_Damage;
	FGameplayTag Debuff_Frequency;
	FGameplayTag Debuff_Duration;
	
	TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;

	/* Ability Tags */
	FGameplayTag Abilities;
	FGameplayTag Abilities_None;
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_Summon;

	FGameplayTag Abilities_Fire_Firebolt;
	
	FGameplayTag Abilities_Lightning_Electrocute;
	
	FGameplayTag Abilities_HitReact;

	FGameplayTag Abilities_Status;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Equipped;
	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Unlocked;

	FGameplayTag Abilities_Type_None;
	FGameplayTag Abilities_Type_Offensive;
	FGameplayTag Abilities_Type_Passive;
	
	
	/* Cooldown Tags */
	FGameplayTag Cooldown_Fire_Firebolt;

	FGameplayTag Cooldown_Lightning_Electrocute;
	
	/* Effect Tags */
	FGameplayTag Effects_HitReact;

	/* Combat Socket Tags */
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_Tail;

	/* Montage Tags */
	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;

	/* Input Tags */
	FGameplayTag InputTag;
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;
	
	/* Player Tags */
	FGameplayTag Player_Block_ClickToMove;
	FGameplayTag Player_Block_CursorTrace;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputReleased;


	
private:

 	static FAuraGameplayTags GameplayTags; 
};