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

	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs; // Map to associate Capture Definitions with GameplayTag
	
	
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

		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		// Filling the map
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, BlockChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, CriticalHitChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, CriticalHitDamageDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, CriticalHitResistanceDef);
		
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, FireResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, LightningResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, ArcaneResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, PhysicalResistanceDef);
		
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

// Core logic for applying this GameplayEffect
void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// Get source and target Ability System Components
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// Get corresponding actors from ASC
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// Get the Damage CurveTable from the Character Class Info of the actor. 
	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	// Get Combat Interfaces from Source and Target actors
	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);

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
	
	// Get Damage Set by Caller Magnitude
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		
		checkf(AuraDamageStatics().TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = AuraDamageStatics().TagsToCaptureDefs[ResistanceTag];

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key);
		
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		DamageTypeValue *= (100.f - Resistance) / 100.f;
		
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
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());
	
	// ArmorPenetration ignores a percentage of the Target's Armor.
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;
	
	// Get the EffectiveArmor Curve and evaluate it with current player level
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetCombatInterface->GetPlayerLevel());
	
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
	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetCombatInterface->GetPlayerLevel());

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
