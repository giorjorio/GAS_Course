// Giorjorio Copyright

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"


class AMagicCircle;
class UNiagaraSystem;
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
	
	UFUNCTION(BlueprintCallable, Category = "Aura|UI")
	void SetCursorVisibilityAndRefresh(bool bShowCursor);
	
	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInstance* DecalMaterial = nullptr);
	
	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();

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
	bool GetCursorPlaneIntersection(const FVector& InPlaneOrigin, const FVector& InPlaneNormal, FVector& OutPlanePoint) const;
	bool GetScreenPositionPlaneIntersection(const FVector2d& ScreenPosition, const FVector& InPlaneOrigin, const FVector& InPlaneNormal, FVector& OutPlanePoint) const;
	
	bool bAutoRunning = false;
	bool bTargeting = false;
	float ControlledPawnHalfHeight = 0.0f;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	float WaitToHeldToMove = 0.3f;
	int32 TargetSplinePointIdx = 0;

	
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
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;

	void AutoRun();

	/*
	 * Show Damage Number
	 */

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircle> MagicCircleClass;
	
	UPROPERTY()
	TObjectPtr<AMagicCircle> MagicCircle;

	
	 void UpdateMagicCircleLocation();

};

