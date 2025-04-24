// Giorjorio Copyright

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_LightningResistance.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UMMC_LightningResistance : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UMMC_LightningResistance();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:

	FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
	FGameplayEffectAttributeCaptureDefinition ResilienceDef;
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
	
};
