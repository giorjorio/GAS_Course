// Giorjorio Copyright


#include "Actor/PointCollection.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/AuraLogChannels.h"
#include "Kismet/KismetMathLibrary.h"

APointCollection::APointCollection()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Pt_0 = CreateDefaultSubobject<USceneComponent>("Pt_0");
	ImmutablePoints.Add(Pt_0);
	SetRootComponent(Pt_0);
	
	Pt_1 = CreateDefaultSubobject<USceneComponent>("Pt_1");
	ImmutablePoints.Add(Pt_1);
	Pt_1->SetupAttachment(GetRootComponent());
	
	Pt_2 = CreateDefaultSubobject<USceneComponent>("Pt_2");
	ImmutablePoints.Add(Pt_2);
	Pt_2->SetupAttachment(GetRootComponent());
	
	Pt_3 = CreateDefaultSubobject<USceneComponent>("Pt_3");
	ImmutablePoints.Add(Pt_3);
	Pt_3->SetupAttachment(GetRootComponent());
	
	Pt_4 = CreateDefaultSubobject<USceneComponent>("Pt_4");
	ImmutablePoints.Add(Pt_4);
	Pt_4->SetupAttachment(GetRootComponent());
	
	Pt_5 = CreateDefaultSubobject<USceneComponent>("Pt_5");
	ImmutablePoints.Add(Pt_5);
	Pt_5->SetupAttachment(GetRootComponent());
	
	Pt_6 = CreateDefaultSubobject<USceneComponent>("Pt_6");
	ImmutablePoints.Add(Pt_6);
	Pt_6->SetupAttachment(GetRootComponent());
	
	Pt_7 = CreateDefaultSubobject<USceneComponent>("Pt_7");
	ImmutablePoints.Add(Pt_7);
	Pt_7->SetupAttachment(GetRootComponent());
	
	Pt_8 = CreateDefaultSubobject<USceneComponent>("Pt_8");
	ImmutablePoints.Add(Pt_8);
	Pt_8->SetupAttachment(GetRootComponent());
	
	Pt_9 = CreateDefaultSubobject<USceneComponent>("Pt_9");
	ImmutablePoints.Add(Pt_9);
	Pt_9->SetupAttachment(GetRootComponent());
	
	Pt_10 = CreateDefaultSubobject<USceneComponent>("Pt_10");
	ImmutablePoints.Add(Pt_10);
	Pt_10->SetupAttachment(GetRootComponent());

}

TArray<FTransform> APointCollection::CalculateAndGetGroundPoints(int32 NumPoints, float YawOverride)
{
	checkf(ImmutablePoints.Num() > 0, TEXT("%hs: The number of Points must be greater than 0."), __FUNCTION__);
 
	if (NumPoints < 1)
	{
		UE_LOG(LogAura, Warning, TEXT("%hs: Called with invalid PointCount (%i), it must be greater than 0."), __FUNCTION__, NumPoints);
		return TArray<FTransform>();
	}
 
	if (NumPoints > ImmutablePoints.Num())
	{
		UE_LOG(LogAura, Warning, TEXT("%hs: Called with invalid PointCount (%i), setting PointCount to the maximum available (%i)."), __FUNCTION__, NumPoints, ImmutablePoints.Num());
		NumPoints = ImmutablePoints.Num();
	}
	
	TArray<FTransform> ArrayCopy;
	
	TArray<AActor*> IgnoredActors;
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(this, IgnoredActors, TArray<AActor*>(), 1500.f, GetActorLocation());
	
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	
	for (const TObjectPtr<USceneComponent>& Pt: ImmutablePoints)
	{
		FTransform PointTransform = Pt->GetComponentTransform();
		
		if (Pt != Pt_0)
		{
			FVector ToPoint = PointTransform.GetLocation() - Pt_0->GetComponentLocation();
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			PointTransform.SetLocation(Pt_0->GetComponentLocation() + ToPoint);
		}
		
		const FVector RaisedLocation = FVector(PointTransform.GetLocation().X, PointTransform.GetLocation().Y, PointTransform.GetLocation().Z + 500.f);
		const FVector LoweredLocation = FVector(PointTransform.GetLocation().X, PointTransform.GetLocation().Y, PointTransform.GetLocation().Z - 500.f);
		
		FHitResult HitResult;
		
		GetWorld()->LineTraceSingleByChannel(HitResult, RaisedLocation, LoweredLocation, ECC_Visibility, CollisionQueryParams);
		
		if (HitResult.bBlockingHit)
		{
			const FVector AdjustedLocation = FVector(PointTransform.GetLocation().X, PointTransform.GetLocation().Y, HitResult.ImpactPoint.Z);
			PointTransform.SetLocation(AdjustedLocation);
			PointTransform.SetRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal).Quaternion());
		
			ArrayCopy.Add(PointTransform);
		
			if (ArrayCopy.Num() >= NumPoints) return ArrayCopy;
		}
	}
	return ArrayCopy;
}

void APointCollection::BeginPlay()
{
	Super::BeginPlay();
	
}


