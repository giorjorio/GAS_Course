// Giorjorio Copyright


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AuraGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"

FString UAuraFireBolt::GetDescription(int32 Level)
{
	
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCoolDown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"
			
			// Level
			"<Small>Level: </><Level>%d</>\n"
			// Manacost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
			"<Default>Launches a bolt of fire, "
			"exploding on impact and dealing: </>"
			
			// Damage
			"<Damage>%d</><Default> fire damage with "
			
			"a chance to burn</>\n\n"),
			
			// Values
			Level, 
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
		// Title
		"<Title>NEXT LEVEL: </>\n\n"
			
		// Level
		"<Small>Level: </><Level>%d</>\n"
		// Manacost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
		// Number of FireBolts
		"<Default>Launches %d bolts of fire, "
		"exploding on impact and dealing: </>"
			
		// Damage
		"<Damage>%d</><Default> fire damage with "
			
		"a chance to burn</>\n\n"),
			
		// Values
		Level, 
		ManaCost, 
		Cooldown,
		FMath::Min(Level, MaxNumberOfProjectiles),
		ScaledDamage);
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 NextLevel)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(NextLevel);
	const float ManaCost = FMath::Abs(GetManaCost(NextLevel));
	const float Cooldown = GetCoolDown(NextLevel);

	return FString::Printf(TEXT(
		// Title
		"<Title>FIRE BOLT</>\n\n"
			
		// Level
		"<Small>Level: </><Level>%d</>\n"
		// Manacost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
		// Number of FireBolts
		"<Default>Launches %d bolts of fire, "
		"exploding on impact and dealing: </>"
			
		// Damage
		"<Damage>%d</><Default> fire damage with "
			
		"a chance to burn</>\n\n"),
			
		// Values
		NextLevel, 
		ManaCost, 
		Cooldown,
		FMath::Min(NextLevel, MaxNumberOfProjectiles),
		ScaledDamage);


}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
	bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(),	SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	
	if (bOverridePitch)	Rotation.Pitch = PitchOverride;
	
	const FVector Forward = Rotation.Vector();
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-ProjectileSpread / 2.f, FVector::UpVector);
	const FVector RightOfSpread = Forward.RotateAngleAxis(ProjectileSpread / 2.f, FVector::UpVector);
	
	//NumberOfProjectiles = FMath::Min(MaxNumberOfProjectiles, GetAbilityLevel());
	
	
	
	if (NumberOfProjectiles > 1)
	{
		const float DeltaSpread = ProjectileSpread / (NumberOfProjectiles - 1) ;
		for (int32 i = 0; i < NumberOfProjectiles; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation+FVector(0,0,10), SocketLocation+FVector(0,0,10)+Direction * 100.f, 5, FLinearColor::Red, 120, 2);

		}
	}
	else
	{
		// Single projectile
		UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation+FVector(0,0,10), SocketLocation+FVector(0,0,10)+Forward * 100.f, 5, FLinearColor::Blue, 120, 2);

	}
	
	
	UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation+Rotation.Vector() * 100.f, 5, FLinearColor::White, 120, 2);
	UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation+LeftOfSpread * 100.f, 5, FLinearColor::Gray, 120, 2);
	UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation+RightOfSpread * 100.f, 5, FLinearColor::Gray, 120, 2);
}
