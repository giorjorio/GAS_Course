// Giorjorio Copyright


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"

// Struct for capturing necessary attributes from the target
struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor); // Declare a capture definition for the Armor attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration); // Declare a capture definition for the ArmorPenetration attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance); // Declare a capture definition for the BlockChance attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance); // Declare a capture definition for the CriticalHitChance attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage); // Declare a capture definition for the CriticalHitDamage attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance); // Declare a capture definition for the CriticalHitResistance attribute

	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance); // Declare a capture definition for the FireResistance attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance); // Declare a capture definition for the LightningResistance attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance); // Declare a capture definition for the ArcaneResistance attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance); // Declare a capture definition for the PhysicalResistance attribute

	
	
	
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false); // Define how to capture Armor from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false) // Define how to capture ArmorPenetration from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false); // Define how to capture BlockChance from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false); // Define how to capture CriticalHitChance from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false) // Define how to capture CriticalHitDamage from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false); // Define how to capture CriticalHitResistance from the target (do not snapshot it)

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false); // Define how to capture FireResistance from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false); // Define how to capture LightningResistance from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false); // Define how to capture ArcaneResistance from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false); // Define how to capture PhysicalResistance from the target (do not snapshot it)
		
	}
};

// Singleton pattern to ensure DamageStatics is initialized once
static const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DStatics;
	return DStatics;
}

// Constructor: Register the Armor attribute as relevant for this execution calculation
UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& Spec, FAggregatorEvaluateParameters EvaluationParameters, TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	
	for (TTuple<FGameplayTag, FGameplayTag> Pair : GameplayTags.DamageTypesToDebuffs)
	{
		const FGameplayTag& DamageType = Pair.Key;
		const FGameplayTag& DebuffType = Pair.Value;
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1.f);
		if (TypeDamage > -.5f) // .5 padding for floating point [im]precision
		{
			// Determine if there was a successfull debuff
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false, -1.f);
			
			float TargetDebuffResistance = 0.f;
			const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageType]; 
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(InTagsToDefs[ResistanceTag], EvaluationParameters, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);
			const float EffectiveDebuffChance = SourceDebuffChance * (100 -TargetDebuffResistance) / 100.f;
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			if (bDebuff)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
				
				UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(ContextHandle, true);
				UAuraAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);
				
				const float DebuffDamage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false, -1.f);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false, -1.f);
				
				UAuraAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
				UAuraAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
				UAuraAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
				
			}
		}
	}
}

// Core logic for applying this GameplayEffect
void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	static TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs; // Map to associate Capture Definitions with GameplayTag
	
	const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
	// Filling the map
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, DamageStatics().ArmorDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, DamageStatics().BlockChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, DamageStatics().CriticalHitChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, DamageStatics().CriticalHitDamageDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, DamageStatics().CriticalHitResistanceDef);
		
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, DamageStatics().LightningResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, DamageStatics().ArcaneResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);
	
	
	// Get source and target Ability System Components
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// Get corresponding actors from ASC
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// Get the Damage CurveTable from the Character Class Info of the actor. 
	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	int32 SourcePlayerLevel = 1;
	if (SourceAvatar && SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	int32 TargetPlayerLevel = 1;
	if (TargetAvatar && TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}
	
	// Get Combat Interfaces from Source and Target actors
	/*ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);*/

	// Get the effect specification, which contains data like tags and modifiers
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	// Creating EffectContextHandle
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	// Extract source and target tags for conditional calculations
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// Prepare evaluation parameters using these tags
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	// Debuff
	
	DetermineDebuff(ExecutionParams, Spec, EvaluationParameters, TagsToCaptureDefs);
	
	
	// Get Damage Set by Caller Magnitude
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		
		checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag, false);
		
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		DamageTypeValue *= (100.f - Resistance) / 100.f;
		
		if (UAuraAbilitySystemLibrary::IsRadialDamage(EffectContextHandle))
		{
			DamageTypeValue = UAuraAbilitySystemLibrary::GetRadialDamageWithFalloff(
				TargetAvatar,
				DamageTypeValue,
				0.f, // Minimum damage at the very edge of the radius
				UAuraAbilitySystemLibrary::GetRadialDamageOrigin(EffectContextHandle),
				UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(EffectContextHandle),
				UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(EffectContextHandle),
				1.f // Falloff (1.f means a linear damage drop-off)
				);
		}
		
		Damage += DamageTypeValue;
	}

	/*
	 * Block Chance
	 */
	// Capture Block Chance on Target, and determine if there was a successful Block
	float TargetBlockChance = 0.f;
	
	// Try to calculate the current value of the target's Armor attribute
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);  // Ensure BlockChance is at least 0 to avoid negative values

	// Determine whether Hit is blocked or not
	const bool bBlocked = FMath::FRandRange(UE_SMALL_NUMBER, 100.0f) <= TargetBlockChance;

	// Pass the information to EffectContextHandle whether Hit is blocked or not
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);
	
	// If Block, halve the damage.
	Damage = bBlocked ? Damage / 2.f : Damage; 

	/*
	 * Armor, Armor Penetration
	*/
	float TargetArmor = 0.f;
	
	// Try to calculate the current value of the target's Armor attribute
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);  // Ensure Armor is at least 0 to avoid negative values

	float SourceArmorPenetration = 0.f;
	
	// Try to calculate the current value of the target's Armor attribute.
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);  // Ensure ArmorPenetration is at least 0 to avoid negative values.
	
	// Get the ArmorPenetration Curve and evaluate it with current player level
	float ArmorPenetrationCoefficient = 0.f; 
	float EffectiveArmorCoefficient = 0.f;
	float CriticalHitResistanceCoefficient = 0.f;

	// Unified check: verify if Character Class Info and the Curve Table are valid
	if (CharacterClassInfo && CharacterClassInfo->DamageCalculationCoefficients)
	{
		// Evaluate Armor Penetration
		if (const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString()))
		{
			ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);
		}
       
		// Evaluate Effective Armor
		if (const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString()))
		{
			EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);
		}

		// Evaluate Critical Hit Resistance
		if (const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString()))
		{
			CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetPlayerLevel);
		}
	}
	
	// ArmorPenetration ignores a percentage of the Target's Armor.
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;
	
	// Armor ignores a percentage of incoming Damage.
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;


	/*
	 * Critical Hit Chance, Bonus Damage, Resistance
	 */
	// Critical Hit Chance
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f); 

	// Critical Hit Damage
	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

	// Critical Hit Resistance
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

	// Critical Hit Resistance reduces a percentage of the Source's Critical Hit Chance.
	const float EffectiveCriticalHitChance = SourceCriticalHitChance * (100 - TargetCriticalHitResistance * CriticalHitResistanceCoefficient) / 100.f;
	
	// Check whether hit is Critical or not 
	const bool bCritical = FMath::FRandRange(UE_SMALL_NUMBER, 100.0f) <= EffectiveCriticalHitChance;

	// Pass the information to EffectContextHandle whether hit is Critical or not 
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCritical);

	// Double the damage plus a bonus if critical hit.
	Damage = bCritical ? Damage * 2.f + SourceCriticalHitDamage : Damage; 
	
	
	// Create the output data: apply this Armor value additively
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);

	// Send this result to the execution output
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
	
}
