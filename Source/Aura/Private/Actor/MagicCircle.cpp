// Giorjorio Copyright


#include "Actor/MagicCircle.h"

#include "Aura/Aura.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Interaction/EnemyInterface.h"

AMagicCircle::AMagicCircle()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
	MagicCircleDecal = CreateDefaultSubobject<UDecalComponent>("MagicCircleDecal");
	MagicCircleDecal->SetupAttachment(GetRootComponent());
	
	TargetingSphere = CreateDefaultSubobject<USphereComponent>("TargetingSphere");
	TargetingSphere->SetupAttachment(GetRootComponent());
	TargetingSphere->SetCollisionResponseToChannel(ECC_Target, ECR_Ignore);

}

void AMagicCircle::BeginPlay()
{
	Super::BeginPlay();
	
	TargetingSphere->OnComponentBeginOverlap.AddDynamic(this, &AMagicCircle::OnSphereBeginOverlap);
	TargetingSphere->OnComponentEndOverlap.AddDynamic(this, &AMagicCircle::OnSphereEndOverlap);
	
}

void AMagicCircle::SetTargetingRadius(float Radius)
{
	TargetingSphere->SetSphereRadius(Radius);
	InitialRadius = Radius;

	if (MagicCircleDecal)
	{
		// FIX: X-axis is the projection depth. Keep it unchanged! Modify only Y and Z.
		MagicCircleDecal->DecalSize = FVector(MagicCircleDecal->DecalSize.X, Radius, Radius);
	}
}

void AMagicCircle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	/*if (MagicCircleDecal)
	{
		if (!MagicCircleMID)
		{
			MagicCircleMID = MagicCircleDecal->CreateDynamicMaterialInstance();
		}
	}*/

	if (InitialRadius > 0.f)
	{
		SetTargetingRadius(InitialRadius);
	}
}

void AMagicCircle::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IEnemyInterface* Enemy = Cast<IEnemyInterface>(OtherActor))
	{
		Enemy->HighlightActor();
	}
}

void AMagicCircle::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IEnemyInterface* Enemy = Cast<IEnemyInterface>(OtherActor))
	{
		Enemy->UnHighlightActor();
	}
}
