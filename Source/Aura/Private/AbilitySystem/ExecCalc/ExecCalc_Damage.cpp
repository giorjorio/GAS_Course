// Giorjorio Copyright


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAttributeSet.h"

// Struct for capturing necessary attributes from the target
struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor); // Declare a capture definition for the Armor attribute
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance); // Declare a capture definition for the BlockChance attribute
	
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false); // Define how to capture Armor from the target (do not snapshot it)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false); // Define how to capture BlockChance from the target (do not snapshot it)
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
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
}

// Core logic for applying this GameplayEffect
void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// Get source and target Ability System Components
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// Get corresponding actors from ASC
	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// Get the effect specification, which contains data like tags and modifiers
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Extract source and target tags for conditional calculations
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// Prepare evaluation parameters using these tags
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	// Get Damage Set by Caller Magnitude
	float Damage = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Damage);

	// Capture Block Chance on Target, and determine if there was a successful Block
	// If Block, halve the damage.
	float TargetBlockChance = 0.f;
	
	// Try to calculate the current value of the target's Armor attribute
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);  // Ensure Armor is at least 0 to avoid negative values

	const bool bBlocked = FMath::FRandRange(UE_SMALL_NUMBER, 100.0f) <= TargetBlockChance;
	Damage = bBlocked ? Damage / 2.f : Damage;
	
	// Create the output data: apply this Armor value additively
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);

	// Send this result to the execution output
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
	
}
