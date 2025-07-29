// Giorjorio Copyright

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"


class UAuraAbilitySystemComponent;
class UAuraInputConfig;
class UDamageTextComponent;
class IEnemyInterface;
class UInputAction;
class UInputMappingContext;
class UNavigationSystemV1;
class USplineComponent;
struct FInputActionValue;


/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:

	/*
	 * Highlight Enemies
	 */

	FHitResult CursorHit;

	void CursorTrace();
	
	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;
	
	/*
	 * Movement
	 */
	
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ShiftAction;

	void Move(const FInputActionValue& InputActionValue);
	
	void ShiftPressed() { bShiftKeyDown = true;}
	void ShiftReleased() { bShiftKeyDown = false;}
	bool bShiftKeyDown = false;

	/*
	 * Abilities Inputs
	 */

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetASC();

	/*
	 * Click to Move
	 */
	
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	float WaitToHeldToMove = 0.3f;
	bool bAutoRunning = false;
	bool bTargeting = false;

	

	UPROPERTY(EditDefaultsOnly, Category = "Click-To-Move")
	float AutoRunAcceptanceRadius = 50.f;

	UPROPERTY(VisibleAnywhere, Category = "Click-To-Move")
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY()
	TObjectPtr<UNavigationSystemV1> NavSystem;

	UPROPERTY(EditDefaultsOnly, Category = "Click-To-Move")
	FVector QueryingExtend = FVector(400.f, 400.f, 250.f);

	UPROPERTY(EditDefaultsOnly, Category = "Click-To-Move")
	bool bDrawDebugEnabled = false;

	void AutoRun();

	/*
	 * Show Damage Number
	 */

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
};

