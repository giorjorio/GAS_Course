// Giorjorio Copyright

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "UObject/Interface.h"
#include "ModifierDependencyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UModifierDependencyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IModifierDependencyInterface
{
	GENERATED_BODY()

// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FOnExternalGameplayModifierDependencyChange* GetOnModifiedDependencyChanged() = 0;


};
