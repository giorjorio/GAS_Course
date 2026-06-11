// Giorjorio Copyright

#include "Player/AuraPlayerController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/Aura.h"
#include "AuraGameplayTags.h"
#include "Components/SplineComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/HUD.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Interaction/CombatInterface.h"
#include "UI/Widget/DamageTextComponent.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);

	NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	AutoRun();
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(GetPawn(), DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();
	
	if (LastActor != ThisActor)
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->HighlightActor();
	}
}

/*
 * Movement
 */

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if  (APawn* ControlledPawn = GetPawn<APawn>())
	{
		bAutoRunning = false;
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector PawnLocation = ControlledPawn->GetActorLocation();
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(PawnLocation, ESplineCoordinateSpace::World);
		const FVector TargetSplinePointLocation = Spline->GetLocationAtSplinePoint(TargetSplinePointIdx,ESplineCoordinateSpace::World);
		FVector WorldDirection = TargetSplinePointLocation - PawnLocation;
		WorldDirection.Z = 0.f;
		//	NOTE: We get the normal after zeroing Z to get a constant movement speed along the XY plane.
		WorldDirection = WorldDirection.GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection);

		const float DistanceToDestination = (LocationOnSpline - TargetSplinePointLocation).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			const bool bNextTargetPointExist = TargetSplinePointIdx < Spline->GetNumberOfSplinePoints() - 1;
			if (bNextTargetPointExist)
			{
				TargetSplinePointIdx++;
			}
			else
			{
				bAutoRunning = false;
			}
		}
		if (bDrawDebugEnabled)
		{
			for (int32 SplinePointIdx = 0; SplinePointIdx < Spline->GetNumberOfSplinePoints(); ++SplinePointIdx)
			{
				const FVector SplinePointLocation = Spline->GetLocationAtSplinePoint(SplinePointIdx,ESplineCoordinateSpace::World);
				if (SplinePointIdx > 0)
				{
					const FVector PreviousSplinePointLocation = Spline->GetLocationAtSplinePoint(SplinePointIdx - 1, ESplineCoordinateSpace::World);
					DrawDebugLine(GetWorld(), PreviousSplinePointLocation, SplinePointLocation, FColor::Red);
				}
				DrawDebugSphere(GetWorld(), SplinePointLocation, 10.f, 12, FColor::Red);
			}
			DrawDebugSphere(GetWorld(), LocationOnSpline, 20.f, 12, FColor::Cyan);

			const FVector LineStart = PawnLocation + WorldDirection.GetSafeNormal() * 50.f;
			const FVector LineEnd = LineStart + WorldDirection * 100.f;
			UKismetSystemLibrary::DrawDebugArrow(this, LineStart, LineEnd, 20.f, FLinearColor::Yellow, 0.f, 4.f);

			DrawDebugSphere(GetWorld(), TargetSplinePointLocation, 20.f, 12, FColor::Yellow);

			UE_LOG(LogTemp, Warning, TEXT("TargetSplinePointIdx: %i, DistanceToDestination: %f"), TargetSplinePointIdx, DistanceToDestination);
		}
	}

	
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = ThisActor ? true : false;
		bAutoRunning = false;
		ControlledPawnHalfHeight = Cast<ICombatInterface>(GetPawn())->GetHalfHeight();
	}
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	if (bTargeting || bShiftKeyDown)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		if (FollowTime >= WaitToHeldToMove)
		{
			bAutoRunning = false;
			if (APawn* ControlledPawn = GetPawn())
			{
				const FVector PawnLocation = ControlledPawn->GetActorLocation();
				FVector PawnBottomLocation = PawnLocation;
				PawnBottomLocation.Z -= ControlledPawnHalfHeight;
				FVector CursorHorizPlaneIntersection;
				const bool bIntersectionFound = GetCursorPlaneIntersection(PawnBottomLocation, FVector::UpVector, CursorHorizPlaneIntersection);
				if (bIntersectionFound)
				{
					FVector WorldDirection = (CursorHorizPlaneIntersection - PawnLocation).GetSafeNormal();
					WorldDirection.Z = 0.f;
					ControlledPawn->AddMovementInput(WorldDirection);

					if (bDrawDebugEnabled)
					{
						DrawDebugSphere(GetWorld(), CursorHorizPlaneIntersection, 20.f, 12, FColor::Green);
						const FVector LineStart = PawnLocation + WorldDirection.GetSafeNormal() * 50.f;
						const FVector LineEnd = LineStart + WorldDirection * 100.f;
						UKismetSystemLibrary::DrawDebugArrow(this, LineStart, LineEnd, 20.f, FLinearColor::Green, 0.f, 4.f);
					}
				}
			}
		}
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}
	
	if (GetASC())
	{
		GetASC()->AbilityInputTagReleased(InputTag);
	}
	
	if (!bTargeting && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			FHitResult NavChannelCursorHitResult;
			GetHitResultUnderCursor(ECC_Navigation, false, NavChannelCursorHitResult);
			if (NavChannelCursorHitResult.bBlockingHit)
			{
				// Projecting a point from the cursor impact point to the NavMesh with a larger-than-default
				// Query Extent, so there are better chances to reach for the NavMesh and return a point,
				// then generating a path from the pawn location to this point (only if found).

				FNavLocation ImpactPointNavLocation;
				// NOTE: Default Query Extend = FVector(50.0f, 50.0f, 250.0f)
				const FNavAgentProperties& NavAgentProps = GetNavAgentPropertiesRef();
				const bool bNavLocationFound = NavSystem->ProjectPointToNavigation(NavChannelCursorHitResult.ImpactPoint, ImpactPointNavLocation, QueryingExtend, &NavAgentProps);

				if (bNavLocationFound)
				{
					UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), ImpactPointNavLocation);
					if (NavPath && NavPath->PathPoints.Num() > 0)
					{
						// So in the case where we would run off into the distance 
						// is actually a case where we had no path points in the array.  
						// So just check for that and only start running if we get at least one path point.
						
						Spline->ClearSplinePoints();
						for (const FVector& PointLoc : NavPath->PathPoints)
						{
							Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
						}
						
						if (Spline->GetNumberOfSplinePoints() > 1)
						{
						 	TargetSplinePointIdx = 1;
						 	bAutoRunning = true;
						}
						else
						{
							bAutoRunning = false;
						}
						const FVector FinalDestination = NavPath->PathPoints.Last();
						
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, FinalDestination);
					}
				}
				if (bDrawDebugEnabled)
				{
					DrawDebugBox(GetWorld(), NavChannelCursorHitResult.ImpactPoint, QueryingExtend, FColor::Silver, false, 3.0f);
					DrawDebugSphere(GetWorld(), ImpactPointNavLocation, 20.f, 12, FColor::Yellow, false, 3.0f);
				}
			}
			
		}
		FollowTime = 0.f;
		bTargeting = false;
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

bool AAuraPlayerController::GetCursorPlaneIntersection(const FVector& InPlaneOrigin, const FVector& InPlaneNormal,
	FVector& OutPlanePoint) const
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->ViewportClient)
	{
		FVector2D MousePosition;
		const bool bMousePositionFound = LocalPlayer->ViewportClient->GetMousePosition(MousePosition);
		if (bMousePositionFound)
		{
			return GetScreenPositionPlaneIntersection(MousePosition, InPlaneOrigin, InPlaneNormal, OutPlanePoint);
		}
	}
	return false;
}

bool AAuraPlayerController::GetScreenPositionPlaneIntersection(const FVector2d& ScreenPosition,
	const FVector& InPlaneOrigin, const FVector& InPlaneNormal, FVector& OutPlanePoint) const
{
	// Early out if we clicked on a HUD hitbox.
	AHUD* HUD = GetHUD();
	if (HUD && HUD->GetHitBoxAtCoordinates(ScreenPosition, true))
	{
		return false;
	}

	FVector WorldOrigin;
	FVector WorldDirection;
	const bool bScreenPositionDeprojected = UGameplayStatics::DeprojectScreenToWorld(this, ScreenPosition, WorldOrigin, WorldDirection);
	if (bScreenPositionDeprojected)
	{
		OutPlanePoint = FMath::LinePlaneIntersection(WorldOrigin, WorldOrigin + WorldDirection * HitResultTraceDistance, InPlaneOrigin, InPlaneNormal);
		return true;
	}
	return false;
}
