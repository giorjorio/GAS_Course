// Giorjorio Copyright


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

#include "Interaction/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (!CombatInterface) return TArray<FVector>();

	// Waiting for the OnDeathDelegate to work perfect
	int32 CurrentMinions = CombatInterface->Execute_GetMinionCount(GetAvatarActorFromActorInfo());
	const float DeltaSpread = SpawnSpread / (NumMinions - CurrentMinions);


	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
	TArray<FVector> SpawnLocations;
	for (int32 i = 0; i < NumMinions - CurrentMinions; i++)
	{
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i + DeltaSpread / 2.f, FVector::UpVector);
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0.f, 0.f, 400.f), ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
		if (Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.ImpactPoint;
		}
		SpawnLocations.Add(ChosenSpawnLocation);
	}
	
	return SpawnLocations;

}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Selection];
}
