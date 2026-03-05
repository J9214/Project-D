#include "AbilitySystem/Abilities/Player/GA_Fire.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Components/Combat/WeaponStateComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Pawn/PDPawnBase.h"
#include "Weapon/PDWeaponBase.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "PDGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/MassAI/MassPerceptionSubsystem.h"
#include "AI/MassAI/MassDamageBridgeSubsystem.h"
#include "AI/MassAI/MassProxyPoolSubsystem.h"

UGA_Fire::UGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
	ActivationPolicy = EPDAbilityActivationPolicy::WhileInputActive;
	bReplicateInputDirectly = true;
}

void UGA_Fire::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	// bKeepFiring = true;
	//
	// if (IsLocallyControlled())
	// {
	// 	StartFireNow();
	// }
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_Fire::ActivateAbility - Binding server target data delegate"));
		BindServerTargetDataDelegate();
	}

	if (IsLocallyControlled())
	{
		bKeepFiring = true;
		StartFireNow();
	}
}

void UGA_Fire::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	bKeepFiring = false;

	if (WaitDelayTask)
	{
		WaitDelayTask->EndTask();
		WaitDelayTask = nullptr;
	}
	
	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
		UnbindServerTargetDataDelegate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Fire::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	bKeepFiring = false;

	if (WaitDelayTask)
	{
		WaitDelayTask->EndTask();
		WaitDelayTask = nullptr;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Fire::HandleServerReceivedTargetData(
	const FGameplayAbilityTargetDataHandle& Data,
	FGameplayTag ActivationTag,
	FGameplayAbilitySpecHandle Handle,
	FPredictionKey ShotKey
)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	UAbilitySystemComponent* OwnerASC = nullptr;
	APDPawnBase* OwnerPawn = nullptr;
	APDWeaponBase* Weapon = nullptr;
	if (!GetOwnerPawnWeapon(OwnerASC, OwnerPawn, Weapon))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const FGameplayAbilityTargetData* Raw = Data.Get(0);
	if (!Raw || Raw->GetScriptStruct() != FGameplayAbilityTargetData_LocationInfo::StaticStruct())
	{
		return;
	}

	const FGameplayAbilityTargetData_LocationInfo* LocationInfo = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(Raw);

	const FVector AimPoint = LocationInfo->TargetLocation.GetTargetingTransform().GetLocation();

	if (!Weapon->CanFire())
	{
		return;
	}
	
	if (Weapon->WeaponData->FireCueTag.IsValid())
	{
		FGameplayCueParameters Params;
		Params.SourceObject = Weapon->WeaponData;
		Params.EffectCauser = Weapon;
		Params.Instigator   = OwnerPawn;

		OwnerASC->ExecuteGameplayCue(Weapon->WeaponData->FireCueTag, Params);
	}

	Weapon->ConsumeAmmo(1);
	ApplyFireCooldownToOwner(Weapon);
	MuzzleTraceAndApplyGE(OwnerPawn, Weapon, AimPoint);
}

void UGA_Fire::StartFireNow()
{
	UAbilitySystemComponent* ASC = nullptr;
	APDPawnBase* OwnerPawn = nullptr;
	APDWeaponBase* Weapon = nullptr;
	if (!GetOwnerPawnWeapon(ASC, OwnerPawn, Weapon))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FireOneShot();

	if (Weapon->GetCurrentFireMode() == EPDWeaponFireMode::FullAuto)
	{
		ScheduleNextShot(Weapon->WeaponData->FireInterval);
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_Fire::ScheduleNextShot(float Interval)
{
	if (!bKeepFiring)
	{
		return;
	}

	if (WaitDelayTask)
	{
		WaitDelayTask->EndTask();
		WaitDelayTask = nullptr;
	}

	WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, FMath::Max(0.01f, Interval));
	if (WaitDelayTask)
	{
		WaitDelayTask->OnFinish.AddDynamic(this, &UGA_Fire::OnWaitDelayFinished);
		WaitDelayTask->ReadyForActivation();
	}
}

void UGA_Fire::OnWaitDelayFinished()
{
	if (!bKeepFiring)
	{
		return;
	}

	FireOneShot();

	UAbilitySystemComponent* ASC = nullptr;
	APDPawnBase* OwnerPawn = nullptr;
	APDWeaponBase* Weapon = nullptr;
	if (!GetOwnerPawnWeapon(ASC, OwnerPawn, Weapon))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (Weapon->GetCurrentFireMode() == EPDWeaponFireMode::FullAuto)
	{
		ScheduleNextShot(Weapon->WeaponData->FireInterval);
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_Fire::FireOneShot()
{
	if (!IsLocallyControlled() || !bKeepFiring)
	{
		return;
	}

	UAbilitySystemComponent* ASC = nullptr;
	APDPawnBase* OwnerPawn = nullptr;
	APDWeaponBase* Weapon = nullptr;
	if (!GetOwnerPawnWeapon(ASC, OwnerPawn, Weapon))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	if (!Weapon->CanFire())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	PlayLocalFireFX(OwnerPawn, Weapon);

	// const UDataAsset_Weapon* WeaponDA = Weapon->WeaponData;
	// if (!WeaponDA)
	// {
	// 	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	// 	return;
	// }
	//
	// const FPDWeaponMontageEntry& Entry = WeaponDA->WeaponMontages.Get(EPDWeaponMontageAction::Fire);
	//
	// UAbilityTask_PlayMontageAndWait* PlayTask =
	// 	UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
	// 		this,
	// 		TEXT("FireMontageTask"),
	// 		Entry.Montage,
	// 		1.f,
	// 		NAME_None,
	// 		Entry.bStopWhenAbilityEnds
	// 	);
	// if (IsValid(PlayTask))
	// {
	// 	PlayTask->ReadyForActivation();
	// }
	
	// {
	// 	FScopedPredictionWindow PW(ASC, true);
	// 	const FPredictionKey PredictKey = ASC->ScopedPredictionKey;
	//
	// 	UWeaponStateComponent* WSC = OwnerPawn->GetWeaponStateComponent();
	// 	if (WSC)
	// 	{
	// 		WSC->ServerRPC_RegisterFireShotKey(PDGameplayTags::InputTag_Weapon_Fire, PredictKey);
	// 	}
	//
	// 	const FVector ViewStart = OwnerPawn->GetPawnViewLocation();
	// 	const FVector AimPoint = CalcLocalAimPoint(OwnerPawn, Weapon);
	// 	const FGameplayAbilityTargetDataHandle TargetData = MakeAimPointTargetData(ViewStart, AimPoint);
	//
	// 	ASC->CallServerSetReplicatedTargetData(
	// 		CurrentSpecHandle,
	// 		PredictKey,
	// 		TargetData,
	// 		FGameplayTag(),
	// 		PredictKey
	// 	);
	// }
	
	const FPredictionKey& ActivationKey = CurrentActivationInfo.GetActivationPredictionKey();
	UE_LOG(LogTemp, Warning, TEXT("UGA_Fire::FireOneShot - ActivationKey: %s"), *ActivationKey.ToString());

	FScopedPredictionWindow PW(ASC, ActivationKey);

	const FVector ViewStart = OwnerPawn->GetPawnViewLocation();
	const FVector AimPoint  = CalcLocalAimPoint(OwnerPawn, Weapon);
	const FGameplayAbilityTargetDataHandle TargetData = MakeAimPointTargetData(ViewStart, AimPoint);

	ASC->CallServerSetReplicatedTargetData(
		CurrentSpecHandle,
		ActivationKey,
		TargetData,
		FGameplayTag(),
		ActivationKey
	);
}

bool UGA_Fire::GetOwnerPawnWeapon(
	UAbilitySystemComponent*& OutASC,
	APDPawnBase*& OutPawn,
	APDWeaponBase*& OutWeapon
)
{
	OutASC = GetAbilitySystemComponentFromActorInfo();
	if (!OutASC)
	{
		return false;
	}
	
	OutPawn = GetPlayerPawnFromActorInfo();
	if (!OutPawn)
	{
		return false;
	}

	UWeaponManageComponent* WMC = OutPawn->GetWeaponManageComponent();
	if (!WMC)
	{
		return false;
	}

	OutWeapon = WMC->GetEquippedWeapon();
	if (!IsValid(OutWeapon) || !OutWeapon->WeaponData)
	{
		return false;
	}

	return true;
}

FVector UGA_Fire::CalcLocalAimPoint(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon) const
{
	UWorld* World = GetWorld();
	if (!World || !OwnerPawn || !Weapon || !Weapon->WeaponData)
	{
		return FVector::ZeroVector;
	}

	const float MaxRange = Weapon->WeaponData->MaxRange;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		const FVector ViewLocation = OwnerPawn->GetPawnViewLocation();
		const FRotator ViewRotation = OwnerPawn->GetBaseAimRotation();
		return ViewLocation + ViewRotation.Vector() * MaxRange;
	}

	int32 SizeX = 0;
	int32 SizeY = 0;
	PC->GetViewportSize(SizeX, SizeY);

	if (SizeX <= 0 || SizeY <= 0)
	{
		FVector CamLocation;
		FRotator CamRotation;
		PC->GetPlayerViewPoint(CamLocation, CamRotation);
		return CamLocation + CamRotation.Vector() * MaxRange;
	}

	const float ScreenX = SizeX * 0.5f;
	const float ScreenY = SizeY * 0.5f;

	FVector WorldOrigin;
	FVector WorldDirection;
	if (!PC->DeprojectScreenPositionToWorld(ScreenX, ScreenY, WorldOrigin, WorldDirection))
	{
		FVector CamLocation;
		FRotator CamRotation;
		PC->GetPlayerViewPoint(CamLocation, CamRotation);
		return CamLocation + CamRotation.Vector() * MaxRange;
	}

	const FVector Start = WorldOrigin;
	const FVector End   = Start + WorldDirection.GetSafeNormal() * MaxRange;
	
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GA_Fire_CameraTrace_Client), false, OwnerPawn);
	Params.AddIgnoredActor(Weapon);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, Params);

	//DrawDebugLine(World, Start, bHit ? Hit.ImpactPoint : End, FColor::Cyan, false, 1.f, 0, 1.f);

	return bHit ? Hit.ImpactPoint : End;
}

void UGA_Fire::MuzzleTraceAndApplyGE(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon, const FVector& AimPoint)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	const float MaxRange = Weapon->WeaponData->MaxRange;

	const FVector MuzzleStart = GetChestShotStart(OwnerPawn);
	FVector FireDir = (AimPoint - MuzzleStart).GetSafeNormal();
	if (FireDir.IsNearlyZero())
	{
		FireDir = OwnerPawn->GetActorForwardVector();
	}

	const FVector MuzzleEnd = MuzzleStart + FireDir * MaxRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GA_Fire_MuzzleTrace_Server), false, OwnerPawn);
	Params.AddIgnoredActor(Weapon);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		MuzzleStart,
		MuzzleEnd,
		ECC_GameTraceChannel1,
		Params
	);

	{
		UMassPerceptionSubsystem* Perception = World->GetSubsystem<UMassPerceptionSubsystem>();
		if (IsValid(Perception) == true)
		{
			const FVector TubeEnd = (bHit == true) ? Hit.ImpactPoint : MuzzleEnd;
			const float TubeLen = (TubeEnd - MuzzleStart).Size();

			Perception->SubmitAimTubeRequest(MuzzleStart, FireDir, TubeLen);
		}
	}

	ApplyWeaponDamageGE(Hit, Weapon);

	OwnerPawn->ClientDrawFireDebug(MuzzleStart, bHit ? Hit.ImpactPoint : MuzzleEnd, bHit, Hit.ImpactPoint);
}

void UGA_Fire::ApplyWeaponDamageGE(const FHitResult& Hit, const APDWeaponBase* Weapon)
{
	if (!Weapon || !Weapon->WeaponData || !Weapon->WeaponData->WeaponDamageGE)
	{
		return;
	}

	AActor* TargetActor = Hit.GetActor();
	if (!IsValid(TargetActor))
	{
		return;
	}

	{
		UWorld* World = GetWorld();
		if (IsValid(World) == true)
		{
			UMassDamageBridgeSubsystem* Bridge = World->GetSubsystem<UMassDamageBridgeSubsystem>();
			if (IsValid(Bridge) == true)
			{
				const float Damage = Weapon->WeaponData->WeaponDamage;

				if (Bridge->TryApplyDamageFromProxyHit(Hit, Damage) == true)
				{
					return;
				}
			}
		}
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	const float Damage = Weapon->WeaponData->WeaponDamage;
	const int32 Level = GetAbilityLevel();

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddHitResult(Hit);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		Weapon->WeaponData->WeaponDamageGE,
		Level,
		Context
	);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_Weapon_Damage, Damage);
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	if (IsValid(Weapon->WeaponData->DestructDamageGE))
	{
		const float DestructDamage = Weapon->WeaponData->DestructDamage;

		SpecHandle = SourceASC->MakeOutgoingSpec(
			Weapon->WeaponData->DestructDamageGE,
			Level,
			Context
		);
		if (!SpecHandle.IsValid())
		{
			return;
		}

		SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_Weapon_DestructDamage, DestructDamage);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void UGA_Fire::ApplyFireCooldownToOwner(const APDWeaponBase* Weapon)
{
	if (!Weapon || !Weapon->WeaponData || !Weapon->WeaponData->FireCooldownGE)
	{
		return;
	}

	const float Interval = FMath::Max(0.01f, Weapon->WeaponData->FireInterval);

	FGameplayEffectSpecHandle CooldownEffect = MakeOutgoingGameplayEffectSpec(
		Weapon->WeaponData->FireCooldownGE,
		GetAbilityLevel()
	);
	if (!CooldownEffect.IsValid())
	{
		return;
	}

	CooldownEffect.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_Weapon_FireInterval, Interval);

	ApplyGameplayEffectSpecToOwner(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		CooldownEffect
	);
}

void UGA_Fire::PlayLocalFireFX(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon)
{
	if (!IsLocallyControlled() || !OwnerPawn || !Weapon || !Weapon->WeaponData)
	{
		return;
	}
	
	const UDataAsset_Weapon* WeaponDA = Weapon->WeaponData;
	
	USceneComponent* MuzzleComp = Weapon->GetMuzzleComponent();
	if (!MuzzleComp)
	{
		return;
	}
	
	if (WeaponDA->FireSound)
	{
		UGameplayStatics::SpawnSoundAttached(
			WeaponDA->FireSound,
			MuzzleComp
		);
	}
	
	if (WeaponDA->MuzzleFlashFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			WeaponDA->MuzzleFlashFX,
			MuzzleComp,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}
}

FVector UGA_Fire::GetChestShotStart(APDPawnBase* OwnerPawn) const
{
	if (USkeletalMeshComponent* Mesh = OwnerPawn->GetSkeletalMeshComponent())
	{
		const FName ChestSocketName = TEXT("spine_04");
		if (Mesh->DoesSocketExist(ChestSocketName))
		{
			const FVector ChestLocation = Mesh->GetSocketLocation(ChestSocketName);
			return ChestLocation + OwnerPawn->GetActorForwardVector() * 30.f;
		}
	}
	
	const float HalfHeight = OwnerPawn->GetSimpleCollisionHalfHeight();
	const FVector Base = OwnerPawn->GetActorLocation();
	const FVector Chest = Base + FVector::UpVector * (HalfHeight * 0.7f);
	return Chest + OwnerPawn->GetActorForwardVector() * 30.f;
}

FGameplayAbilityTargetDataHandle UGA_Fire::MakeAimPointTargetData(const FVector& CameraStart, const FVector& AimPoint)
{
	FGameplayAbilityTargetDataHandle Handle;

	FGameplayAbilityTargetData_LocationInfo* Loc = new FGameplayAbilityTargetData_LocationInfo();
	Loc->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	Loc->SourceLocation.LiteralTransform = FTransform(CameraStart);

	Loc->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	Loc->TargetLocation.LiteralTransform = FTransform(AimPoint);

	Handle.Add(Loc);

	return Handle;
}

void UGA_Fire::BindServerTargetDataDelegate()
{
	if (bServerDelegateBound)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	const FPredictionKey& ActivationKey = CurrentActivationInfo.GetActivationPredictionKey();
	UE_LOG(LogTemp, Warning, TEXT("UGA_Fire::BindServerTargetDataDelegate - Retrieved ASC and ActivationKey. ActivationKey: %s"), *ActivationKey.ToString());

	auto& Delegate = ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, ActivationKey);
	if (Delegate.IsBound())
	{
		bServerDelegateBound = true;
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UGA_Fire::BindServerTargetDataDelegate - Binding server target data delegate."));
	ServerTDDelegateHandle = Delegate.AddUObject(this, &UGA_Fire::OnServerTargetDataReceived);
	bServerDelegateBound = true;

	ASC->CallReplicatedTargetDataDelegatesIfSet(CurrentSpecHandle, ActivationKey);
}

void UGA_Fire::UnbindServerTargetDataDelegate()
{
	if (!bServerDelegateBound)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		const FPredictionKey& ActivationKey = CurrentActivationInfo.GetActivationPredictionKey();
		UE_LOG(LogTemp, Warning, TEXT("UGA_Fire::UnbindServerTargetDataDelegate - Retrieved ASC and ActivationKey for unbinding. ActivationKey: %s"), *ActivationKey.ToString());
		ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, ActivationKey).Remove(ServerTDDelegateHandle);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UGA_Fire::UnbindServerTargetDataDelegate - Unbound server target data delegate."));

	bServerDelegateBound = false;
	ServerTDDelegateHandle.Reset();
}

void UGA_Fire::OnServerTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Tag)
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	const FPredictionKey& ActivationKey = CurrentActivationInfo.GetActivationPredictionKey();
	UE_LOG(LogTemp, Warning, TEXT("UGA_Fire::OnServerTargetDataReceived - Retrieved ASC and ActivationKey for processing received data. ActivationKey: %s"), *ActivationKey.ToString());
	ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, ActivationKey);

	HandleServerReceivedTargetData_Internal(Data);
}

void UGA_Fire::HandleServerReceivedTargetData_Internal(const FGameplayAbilityTargetDataHandle& Data)
{
	UAbilitySystemComponent* OwnerASC = nullptr;
	APDPawnBase* OwnerPawn = nullptr;
	APDWeaponBase* Weapon = nullptr;
	if (!GetOwnerPawnWeapon(OwnerASC, OwnerPawn, Weapon))
	{
		return;
	}

	const FGameplayAbilityTargetData* Raw = Data.Get(0);
	if (!Raw || Raw->GetScriptStruct() != FGameplayAbilityTargetData_LocationInfo::StaticStruct())
	{
		return;
	}

	const auto* LocationInfo = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(Raw);
	const FVector AimPoint = LocationInfo->TargetLocation.GetTargetingTransform().GetLocation();

	if (!Weapon->CanFire())
	{
		return;
	}

	if (Weapon->WeaponData && Weapon->WeaponData->FireCueTag.IsValid())
	{
		FGameplayCueParameters Params;
		Params.SourceObject = Weapon->WeaponData;
		Params.EffectCauser = Weapon;
		Params.Instigator   = OwnerPawn;
		OwnerASC->ExecuteGameplayCue(Weapon->WeaponData->FireCueTag, Params);
	}

	UE_LOG(LogTemp, Warning, TEXT("UGA_Fire::HandleServerReceivedTargetData_Internal - Firing weapon. AimPoint: %s"), *AimPoint.ToString());
	Weapon->ConsumeAmmo(1);
	ApplyFireCooldownToOwner(Weapon);
	MuzzleTraceAndApplyGE(OwnerPawn, Weapon, AimPoint);
}