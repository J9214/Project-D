#include "AbilitySystem/Abilities/Player/GA_Flash.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilitySystemComponent.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"
#include "Pawn/PDPawnBase.h"
#include "Weapon/PDThrowableProjectile.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"

UGA_Flash::UGA_Flash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_Flash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	// APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	// if (!OwnerPawn || !FlashData)
	// {
	// 	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	// 	return;
	// }
	//
	// if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	// {
	// 	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	// 	return;
	// }
	//
	// if (HasAuthority(&ActivationInfo))
	// {
	// 	SpawnFlashProjectile(OwnerPawn);
	// }
	//
	// EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	
	APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	if (!OwnerPawn || !FlashData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	const FGameplayAbilitySpecHandle SpecHandle = GetCurrentAbilitySpecHandle();
	const FPredictionKey PredictionKey = GetCurrentActivationInfo().GetActivationPredictionKey();

	if (ActorInfo->IsNetAuthority())
	{
		ASC->AbilityTargetDataSetDelegate(SpecHandle, PredictionKey).AddUObject(this, &UGA_Flash::OnFlashTargetDataReady);
		ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, PredictionKey);
	}

	if (ActorInfo->IsLocallyControlled())
	{
		FVector ViewLoc;
		FRotator ViewRot;
		OwnerPawn->GetActorEyesViewPoint(ViewLoc, ViewRot);

		const FVector TraceStart = ViewLoc;
		const FVector TraceEnd = TraceStart + ViewRot.Vector() * 3000.f;

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(FlashAimTrace), false, OwnerPawn);
		Params.AddIgnoredActor(OwnerPawn);

		const bool bHit = OwnerPawn->GetWorld()->LineTraceSingleByChannel(
			Hit,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			Params
		);

		if (!bHit)
		{
			Hit.TraceStart = TraceStart;
			Hit.TraceEnd = TraceEnd;
			Hit.Location = TraceEnd;
			Hit.ImpactPoint = TraceEnd;
		}

		SendFlashTargetData(Hit);
	}
}

void UGA_Flash::SendFlashTargetData(const FHitResult& HitResult)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(new FGameplayAbilityTargetData_SingleTargetHit(HitResult));

	ASC->ServerSetReplicatedTargetData(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		ASC->ScopedPredictionKey
	);
}

void UGA_Flash::OnFlashTargetDataReady(
	const FGameplayAbilityTargetDataHandle& DataHandle,
	FGameplayTag ActivationTag
)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	ASC->ConsumeClientReplicatedTargetData(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey()
	);

	APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	if (!OwnerPawn)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	SpawnFlashProjectileFromData(OwnerPawn, DataHandle);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Flash::SpawnFlashProjectileFromData(
	APDPawnBase* OwnerPawn,
	const FGameplayAbilityTargetDataHandle& DataHandle
)
{
	if (!OwnerPawn || !FlashData || !FlashData->ProjectileClass || DataHandle.Num() <= 0)
	{
		return;
	}

	const FGameplayAbilityTargetData* TargetData = DataHandle.Get(0);
	if (!TargetData)
	{
		return;
	}

	const FGameplayAbilityTargetData_SingleTargetHit* HitData =
		static_cast<const FGameplayAbilityTargetData_SingleTargetHit*>(TargetData);
	if (!HitData)
	{
		return;
	}

	const FHitResult& HitResult = HitData->HitResult;

	UWorld* World = OwnerPawn->GetWorld();
	if (!World)
	{
		return;
	}

	FVector Start = OwnerPawn->GetActorLocation();
	if (USkeletalMeshComponent* Mesh = OwnerPawn->GetSkeletalMeshComponent())
	{
		if (Mesh->DoesSocketExist(TEXT("ThrowableHand")))
		{
			Start = Mesh->GetSocketLocation(TEXT("ThrowableHand"));
		}
	}

	FVector TargetPoint = HitResult.ImpactPoint;
	if (TargetPoint.IsNearlyZero())
	{
		TargetPoint = HitResult.Location;
	}
	if (TargetPoint.IsNearlyZero())
	{
		TargetPoint = HitResult.TraceEnd;
	}

	FVector ThrowDir = (TargetPoint - Start).GetSafeNormal();
	if (ThrowDir.IsNearlyZero())
	{
		ThrowDir = OwnerPawn->GetActorForwardVector();
	}

	const FVector Velocity = ThrowDir * FlashData->MaxThrowSpeed;

	FActorSpawnParameters Params;
	Params.Owner = OwnerPawn;
	Params.Instigator = OwnerPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APDThrowableProjectile* Projectile = World->SpawnActor<APDThrowableProjectile>(
		FlashData->ProjectileClass,
		Start,
		ThrowDir.Rotation(),
		Params
	);

	if (Projectile)
	{
		Projectile->InitFromData(FlashData, Start, Velocity);
	}
}

void UGA_Flash::SpawnFlashProjectile(APDPawnBase* OwnerPawn)
{
	if (!OwnerPawn || !FlashData || !FlashData->ProjectileClass)
	{
		return;
	}

	UWorld* World = OwnerPawn->GetWorld();
	if (!World)
	{
		return;
	}

	const FRotator AimRot = OwnerPawn->GetBaseAimRotation();
	const FVector Forward = AimRot.Vector();
	const FVector Up = FVector::UpVector;
	
	FVector Start = OwnerPawn->GetActorLocation();
	if (USkeletalMeshComponent* Mesh = OwnerPawn->GetSkeletalMeshComponent())
	{
		if (Mesh->DoesSocketExist(TEXT("ThrowableHand")))
		{
			Start = Mesh->GetSocketLocation("ThrowableHand");
		}
	}

	const FVector Velocity = Forward * FlashData->MaxThrowSpeed;

	FActorSpawnParameters Params;
	Params.Owner = OwnerPawn;
	Params.Instigator = OwnerPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APDThrowableProjectile* Projectile = World->SpawnActor<APDThrowableProjectile>(
		FlashData->ProjectileClass,
		Start,
		Forward.Rotation(),
		Params
	);

	if (Projectile)
	{
		Projectile->InitFromData(FlashData, Start, Velocity);
	}
}