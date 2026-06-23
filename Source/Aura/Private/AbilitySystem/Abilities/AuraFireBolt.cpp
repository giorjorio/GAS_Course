// Giorjorio Copyright


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

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
	NumberOfProjectiles = FMath::Min(GetAbilityLevel(), MaxNumberOfProjectiles);
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpaceRotators(Forward, FVector::UpVector, ProjectileSpread, NumberOfProjectiles);
	
	for (const FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());
		
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		if (bLaunchHomingProjectiles)
		{
			// SCENARIO 1: We have a valid target (clicked on an enemy)
			if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
			{
				// Check if the target is still alive
				if (!ICombatInterface::Execute_IsDead(HomingTarget))
				{
					// The enemy is ALIVE - enable homing towards it
					Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
					Projectile->ProjectileMovement->bIsHomingProjectile = true;
					Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
				}
				else
				{
					// The enemy is DEAD. 
					// We DO NOT create a floating magnet to avoid the "orbiting" bug.
					// We simply disable homing and let the projectile fly straight.
					Projectile->ProjectileMovement->bIsHomingProjectile = false;
				}
			}
			// SCENARIO 2: There was no initial target (clicked on the ground)
			else
			{
				Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(Projectile);
				Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
				
				/* ARCHITECTURE NOTE: 
				Standard Unreal Engine practice requires calling RegisterComponent() after NewObject().
				However, registration is intentionally omitted here to optimize performance.
				Registration incurs system overhead (initializing physics, rendering, and adding to the scene graph),
				whereas UProjectileMovementComponent solely requires access to the transformation matrix (FVector) 
				in RAM for trajectory calculation.
				
				ARCHITECTURAL ALTERNATIVES:
				1. Pre-allocation: Create the USceneComponent in the projectile's constructor via 
				   CreateDefaultSubobject. Move this existing component as needed, avoiding runtime NewObject calls.
				2. Custom Movement Component: Inherit from UProjectileMovementComponent, add an FVector 
				   target variable, and override ComputeHomingAcceleration() to handle vector-based 
				   homing directly, completely eliminating the need for dummy components. */
			
				Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
				Projectile->ProjectileMovement->bIsHomingProjectile = true;
				Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
			}
		}
		Projectile->FinishSpawning(SpawnTransform);
	}
	
	

}
