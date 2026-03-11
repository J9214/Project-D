#include "AbilitySystem/Abilities/Player/GA_Throw.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilitySystemComponent.h"
#include "Pawn/PDPawnBase.h"
#include "TargetActor/TA_ThrowableTrajectory.h"
#include "Weapon/FPDTargetData_Throwable.h"
#include "Weapon/PDThrowableItemBase.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Weapon/PDThrowableProjectile.h"

UGA_Throw::UGA_Throw()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	bReplicateInputDirectly = true;
}

UWeaponManageComponent* UGA_Throw::GetWMC()
{
	APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	if (!OwnerPawn)
	{
		return nullptr;
	}
	
	return OwnerPawn->GetWeaponManageComponent();
}

UDataAsset_Throwable* UGA_Throw::GetEquippedThrowableDA()
{
	UWeaponManageComponent* WMC = GetWMC();
	if (!WMC)
	{
		return nullptr;
	}
	
	APDThrowableItemBase* Item = Cast<APDThrowableItemBase>(WMC->GetEquippedThrowable());
	if (!Item)
	{
		return nullptr;
	}
	
	return Item->GetThrowableData();
}

void UGA_Throw::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	UWeaponManageComponent* WMC = GetWMC();
	UDataAsset_Throwable* DA = GetEquippedThrowableDA();
	if (!WMC || !DA)
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

	const FPredictionKey PredictionKey = ActivationInfo.GetActivationPredictionKey();
	
	if (HasAuthority(&ActivationInfo))
	{
		ASC->AbilityTargetDataSetDelegate(Handle, PredictionKey).AddUObject(this, &UGA_Throw::OnTargetDataReady);
		ASC->AbilityTargetDataCancelledDelegate(Handle, PredictionKey).AddUObject(this, &UGA_Throw::OnTargetDataCancelled);
		ASC->CallReplicatedTargetDataDelegatesIfSet(Handle, PredictionKey);
	}
	
	if (IsLocallyControlled())
	{
		if (!TargetActorClass)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
	
		WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(
			this,
			NAME_None,
			EGameplayTargetingConfirmation::Custom,
			TargetActorClass
		);
		if (!WaitTargetDataTask)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
	
		AGameplayAbilityTargetActor* GenericActor = nullptr;
		if (WaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, GenericActor))
		{
			CachedTA = Cast<ATA_ThrowableTrajectory>(GenericActor);
			if (CachedTA)
			{
				CachedTA->SetThrowableData(DA);
			}

			WaitTargetDataTask->FinishSpawningActor(this, GenericActor);
		}
		
		WaitTargetDataTask->ReadyForActivation();
	}
}

void UGA_Throw::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	
	if (!IsLocallyControlled())
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !CachedTA)
	{
		if (WaitTargetDataTask)
		{
			WaitTargetDataTask->ExternalCancel();
		}
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	FGameplayAbilityTargetDataHandle DataHandle = CachedTA->MakeTargetData();
	if (DataHandle.Num() <= 0)
	{
		if (WaitTargetDataTask)
		{
			WaitTargetDataTask->ExternalCancel();
		}
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	const FPredictionKey PredKey = ActivationInfo.GetActivationPredictionKey();
	
	ASC->ServerSetReplicatedTargetData(
		Handle,
		PredKey,
		DataHandle,
		FGameplayTag(),
		ASC->ScopedPredictionKey
	);
	
	if (CachedTA)
	{
		CachedTA->Destroy();
	}
	
	CachedTA = nullptr;
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGA_Throw::OnTargetDataCancelled()
{
	if (CachedTA)
	{
		CachedTA->Destroy();
		CachedTA = nullptr;
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Throw::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	if (CachedTA)
	{
		CachedTA->Destroy();
		CachedTA = nullptr;
	}
	
	if (HasAuthority(&CurrentActivationInfo))
	{
		UDataAsset_Throwable* DA = GetEquippedThrowableDA();
		if (!DA || !DA->ProjectileClass)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}

		if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}

		if (DataHandle.Num() <= 0)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}

		const FGameplayAbilityTargetData* Raw = DataHandle.Get(0);
		if (!Raw || Raw->GetScriptStruct() != FPDTargetData_Throwable::StaticStruct())
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}
		
		const FPDTargetData_Throwable* TargetData = (Raw ? static_cast<const FPDTargetData_Throwable*>(Raw) : nullptr);
		if (!TargetData)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}

		SpawnProjectile(*TargetData, DA);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Throw::SpawnProjectile(const FPDTargetData_Throwable& TD, UDataAsset_Throwable* DA)
{
	if (!DA || !DA->ProjectileClass)
	{
		return;
	}
	
	APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	if (!OwnerPawn)
	{
		return;
	}
	
	UWorld* World = OwnerPawn->GetWorld();
	if (!World)
	{
		return;
	}
	
	FActorSpawnParameters Params;
	Params.Owner = OwnerPawn;
	Params.Instigator = OwnerPawn;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector SpawnLoc = TD.StartLocation;
	const FRotator SpawnRot = TD.InitialVelocity.Rotation();

	APDThrowableProjectile* Projectile = World->SpawnActor<APDThrowableProjectile>(DA->ProjectileClass, SpawnLoc, SpawnRot, Params);
	if (!Projectile)
	{
		return;
	}
	
	Projectile->InitFromData(DA, TD.StartLocation, TD.InitialVelocity);
}