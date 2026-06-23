// Giorjorio Copyright


#include "AbilitySystem/Abilities/AuraBeamSpell.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UAuraBeamSpell::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
	
}

void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if(OwnerCharacter->Implements<UCombatInterface>())
	{
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);
			FHitResult HitResult;
			
			// this could be an FName member variable on a Weapon class, so it could be subclassed and used for enemies as well 
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));
			
			UKismetSystemLibrary::SphereTraceSingle(OwnerCharacter,
				SocketLocation,
				BeamTargetLocation,
				3.f,
				TraceTypeQuery1,
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None,
				HitResult,
				true);
			
			if (HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
	{
		if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UAuraBeamSpell::PrimaryTargetDied))
		{
			CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UAuraBeamSpell::PrimaryTargetDied);
		}
	}
}

void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);
	
	TArray<AActor*> OverlappingActors;
	// Radius could not be hardcoded, but can be mathematically computed instead based on Ability Level
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f, // Radius could not be hardcoded, but can be mathematically computed instead based on Ability Level
		MouseHitActor->GetActorLocation());
	
	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);
	//int32 NumAdditionalTargets = MaxNumShockTargets;
	
	UAuraAbilitySystemLibrary::GetClosestTargets(NumAdditionalTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());

	AdditionalTargets = OutAdditionalTargets;
	
	for (AActor* Target : AdditionalTargets)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UAuraBeamSpell::AdditionalTargetDied))
			{
				CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UAuraBeamSpell::AdditionalTargetDied);
			}
		}
	}
}

void UAuraBeamSpell::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 1. Отвязываем ОСНОВНУЮ цель (MouseHitActor)
	if (MouseHitActor && MouseHitActor->Implements<UCombatInterface>())
	{
		ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor);
		if (CombatInterface)
		{
			// Используем RemoveDynamic вместо AddDynamic. 
			// Замени PrimaryTargetDied на то название функции, которым ты привязывал основную цель
			CombatInterface->GetOnDeathDelegate().RemoveDynamic(this, &UAuraBeamSpell::PrimaryTargetDied);
		}
	}

	// 2. Отвязываем ВТОРОСТЕПЕННЫЕ цели
	// Важно: OutAdditionalTargets в твоем коде - это локальная переменная функции. 
	// Тебе нужно, чтобы этот массив был сохранен в классе (например, TArray<AActor*> AdditionalTargets; в .h файле)
	for (AActor* Target : AdditionalTargets) 
	{
		if (Target && Target->Implements<UCombatInterface>())
		{
			ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target);
			if (CombatInterface)
			{
				CombatInterface->GetOnDeathDelegate().RemoveDynamic(this, &UAuraBeamSpell::AdditionalTargetDied);
			}
		}
	}

	// Очищаем массив, чтобы при следующем касте он был пустым
	AdditionalTargets.Empty();
	MouseHitActor = nullptr;

	// Обязательно вызываем базовый EndAbility
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
