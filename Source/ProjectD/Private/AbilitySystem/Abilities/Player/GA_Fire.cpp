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
#include "Skill/PDDamageableSkillActor.h"
#include "Chaos/PDDestructibleObject.h"

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
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ActorInfo && ActorInfo->IsNetAuthority())
	{
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
	
	const FPredictionKey& ActivationKey = CurrentActivationInfo.GetActivationPredictionKey();
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
	const FVector End = Start + WorldDirection.GetSafeNormal() * MaxRange;

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(OwnerPawn);
	IgnoredActors.Add(Weapon);
	
	constexpr int32 MaxRetraceCount = 8;

	for (int32 Count = 0; Count < MaxRetraceCount; ++Count)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(GA_Fire_CameraTrace_Client), false, OwnerPawn);

		for (AActor* IgnoredActor : IgnoredActors)
		{
			if (IsValid(IgnoredActor))
			{
				Params.AddIgnoredActor(IgnoredActor);
			}
		}

		FHitResult Hit;
		const bool bHit = World->LineTraceSingleByChannel(
			Hit,
			Start,
			End,
			ECC_GameTraceChannel1,
			Params
		);

		if (!bHit)
		{
			return End;
		}

		AActor* HitActor = Hit.GetActor();
		if (!IsValid(HitActor))
		{
			return End;
		}

		if (IsFriendlyShield(OwnerPawn, HitActor))
		{
			IgnoredActors.AddUnique(HitActor);
			continue;
		}

		return Hit.ImpactPoint;
	}

	return End;

	// const FVector Start = WorldOrigin;
	// const FVector End   = Start + WorldDirection.GetSafeNormal() * MaxRange;
	//
	// FCollisionQueryParams Params(SCENE_QUERY_STAT(GA_Fire_CameraTrace_Client), false, OwnerPawn);
	// Params.AddIgnoredActor(Weapon);
	//
	// FHitResult Hit;
	// const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, Params);
	//
	// return bHit ? Hit.ImpactPoint : End;
}

void UGA_Fire::MuzzleTraceAndApplyGE(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon, const FVector& AimPoint)
{
	if (!OwnerPawn || !Weapon || !Weapon->WeaponData)
	{
		return;
	}

	if (Weapon->IsMultiBulletWeapon())
	{
		TraceMultiBulletShot(OwnerPawn, Weapon, AimPoint);
	}
	else
	{
		TraceSingleShot(OwnerPawn, Weapon, AimPoint);
	}
}

void UGA_Fire::ApplyWeaponDamageGE(const FHitResult& Hit, const APDWeaponBase* Weapon, float DamageValue)
{
	if (!Weapon || !Weapon->WeaponData || !Weapon->WeaponData->WeaponDamageGE)
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (!IsValid(HitActor))
	{
		return;
	}
	
	// Mass AI
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

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddHitResult(Hit);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		Weapon->WeaponData->WeaponDamageGE,
		GetAbilityLevel(),
		ContextHandle
	);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_Weapon_Damage, DamageValue);
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	
	if (IsValid(Weapon->WeaponData->DestructDamageGE))
	{
		const float DestructDamage = Weapon->WeaponData->DestructDamage;
	
		SpecHandle = SourceASC->MakeOutgoingSpec(
			Weapon->WeaponData->DestructDamageGE,
			GetAbilityLevel(),
			ContextHandle
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

	auto& Delegate = ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, ActivationKey);
	if (Delegate.IsBound())
	{
		bServerDelegateBound = true;
		return;
	}

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
		ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, ActivationKey).Remove(ServerTDDelegateHandle);
	}
	
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

	Weapon->ConsumeAmmo(1);
	ApplyFireCooldownToOwner(Weapon);
	MuzzleTraceAndApplyGE(OwnerPawn, Weapon, AimPoint);
}

void UGA_Fire::TraceSingleShot(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon, const FVector& AimPoint)
{
	UWorld* World = GetWorld();
	if (!World || !OwnerPawn || !Weapon || !Weapon->WeaponData)
	{
		return;
	}

	const FVector TraceStart = GetChestShotStart(OwnerPawn);
	FVector FireDir = (AimPoint - TraceStart).GetSafeNormal();
	if (FireDir.IsNearlyZero())
	{
		FireDir = OwnerPawn->GetActorForwardVector();
	}

	const float MaxRange = Weapon->WeaponData->MaxRange;
	const FVector TraceEnd = TraceStart + FireDir * MaxRange;

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(OwnerPawn);
	IgnoredActors.Add(Weapon);

	bool bHasBlockingHit = false;
	FHitResult FinalHit;

	{
		UMassPerceptionSubsystem* Perception = World->GetSubsystem<UMassPerceptionSubsystem>();
		if (IsValid(Perception))
		{
			const FVector TubeEnd = TraceEnd;
			const float TubeLen = (TubeEnd - TraceStart).Size();

			Perception->SubmitAimTubeRequest(TraceStart, FireDir, TubeLen);
		}
	}

	constexpr int32 MaxRetraceCount = 8;

	for (int32 Count = 0; Count < MaxRetraceCount; ++Count)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(FireSingleTrace), false, OwnerPawn);

		for (AActor* IgnoredActor : IgnoredActors)
		{
			if (IsValid(IgnoredActor))
			{
				Params.AddIgnoredActor(IgnoredActor);
			}
		}

		FHitResult Hit;
		const bool bHit = World->LineTraceSingleByChannel(
			Hit,
			TraceStart,
			TraceEnd,
			ECC_GameTraceChannel1,
			Params
		);

		if (!bHit)
		{
			break;
		}

		AActor* HitActor = Hit.GetActor();
		if (!IsValid(HitActor))
		{
			break;
		}

		if (IsFriendlyShield(OwnerPawn, HitActor))
		{
			IgnoredActors.AddUnique(HitActor);
			continue;
		}

		const EBulletHitDecision Decision = EvaluateHitDecision(OwnerPawn, HitActor);

		if (Decision == EBulletHitDecision::Skip)
		{
			IgnoredActors.AddUnique(HitActor);
			continue;
		}

		FinalHit = Hit;
		bHasBlockingHit = true;

		if (Decision == EBulletHitDecision::BlockAndDamage)
		{
			ApplyWeaponDamageGE(Hit, Weapon, Weapon->WeaponData->WeaponDamage);
		}

		break;
	}


#if ENABLE_DRAW_DEBUG
	OwnerPawn->ClientDrawFireDebug(
		TraceStart,
		bHasBlockingHit ? FinalHit.ImpactPoint : TraceEnd,
		bHasBlockingHit,
		bHasBlockingHit ? FinalHit.ImpactPoint : FVector::ZeroVector
	);
#endif
}

void UGA_Fire::TraceMultiBulletShot(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon, const FVector& AimPoint)
{
	UWorld* World = GetWorld();
	if (!World || !OwnerPawn || !Weapon || !Weapon->WeaponData)
	{
		return;
	}

	const FVector TraceStart = GetChestShotStart(OwnerPawn);

	FVector BaseDir = (AimPoint - TraceStart).GetSafeNormal();
	if (BaseDir.IsNearlyZero())
	{
		BaseDir = OwnerPawn->GetActorForwardVector();
	}

	const int32 BulletCount = Weapon->GetBulletsPerShot();
	const float SpreadHalfAngleDeg = Weapon->GetSpreadHalfAngleDeg();
	const bool bIncludeCenterBullet = Weapon->WeaponData->SpreadConfig.bIncludeCenterBullet;

	const TArray<FVector> BulletDirs = BuildBulletDirections(
		BaseDir,
		BulletCount,
		SpreadHalfAngleDeg,
		bIncludeCenterBullet
	);

	const float MaxRange = Weapon->WeaponData->MaxRange;
	const float BulletDamage = GetDamagePerBullet(Weapon);
	constexpr int32 MaxRetraceCount = 8;
	
	{
		UMassPerceptionSubsystem* Perception = World->GetSubsystem<UMassPerceptionSubsystem>();
		if (IsValid(Perception))
		{
			const FVector TubeEnd = TraceStart + BaseDir * MaxRange;
			const float TubeLen = (TubeEnd - TraceStart).Size();

			Perception->SubmitAimTubeRequest(TraceStart, BaseDir, TubeLen);
		}
	}

	for (const FVector& Dir : BulletDirs)
	{
		const FVector TraceEnd = TraceStart + Dir * MaxRange;

		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(OwnerPawn);
		IgnoredActors.Add(Weapon);

		bool bHasBlockingHit = false;
		FHitResult FinalHit;

		for (int32 Count = 0; Count < MaxRetraceCount; ++Count)
		{
			FCollisionQueryParams Params(SCENE_QUERY_STAT(FireShotgunTrace), false, OwnerPawn);

			for (AActor* IgnoredActor : IgnoredActors)
			{
				if (IsValid(IgnoredActor))
				{
					Params.AddIgnoredActor(IgnoredActor);
				}
			}

			FHitResult Hit;
			const bool bHit = World->LineTraceSingleByChannel(
				Hit,
				TraceStart,
				TraceEnd,
				ECC_GameTraceChannel1,
				Params
			);

			if (!bHit)
			{
				break;
			}

			AActor* HitActor = Hit.GetActor();
			if (!IsValid(HitActor))
			{
				break;
			}

			if (IsFriendlyShield(OwnerPawn, HitActor))
			{
				IgnoredActors.AddUnique(HitActor);
				continue;
			}

			const EBulletHitDecision Decision = EvaluateHitDecision(OwnerPawn, HitActor);

			if (Decision == EBulletHitDecision::Skip)
			{
				IgnoredActors.AddUnique(HitActor);
				continue;
			}

			FinalHit = Hit;
			bHasBlockingHit = true;

			if (Decision == EBulletHitDecision::BlockAndDamage)
			{
				ApplyWeaponDamageGE(Hit, Weapon, BulletDamage);
			}

			break;
		}

#if ENABLE_DRAW_DEBUG
		OwnerPawn->ClientDrawFireDebug(
			TraceStart,
			bHasBlockingHit ? FinalHit.ImpactPoint : TraceEnd,
			bHasBlockingHit,
			bHasBlockingHit ? FinalHit.ImpactPoint : FVector::ZeroVector
		);
#endif
	}
}

TArray<FVector> UGA_Fire::BuildBulletDirections(
	const FVector& BaseDir,
	int32 BulletCount,
	float SpreadHalfAngleDeg,
	bool bIncludeCenterBullet
) const
{
	TArray<FVector> Result;
	Result.Reserve(BulletCount);

	if (BulletCount <= 0)
	{
		return Result;
	}

	FVector Forward = BaseDir.GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		return Result;
	}

	FVector Right = FVector::CrossProduct(Forward, FVector::UpVector);
	if (Right.IsNearlyZero())
	{
		Right = FVector::CrossProduct(Forward, FVector::RightVector);
	}
	Right.Normalize();

	const FVector Up = FVector::CrossProduct(Right, Forward).GetSafeNormal();

	const float HalfAngleRad = FMath::DegreesToRadians(FMath::Max(0.f, SpreadHalfAngleDeg));
	const float DiscRadius = FMath::Tan(HalfAngleRad);

	int32 StartIndex = 0;

	if (bIncludeCenterBullet && BulletCount > 0)
	{
		Result.Add(Forward);
		StartIndex = 1;
	}

	const int32 RemainingCount = BulletCount - StartIndex;
	if (RemainingCount <= 0)
	{
		return Result;
	}

	const float GoldenAngle = PI * (3.f - FMath::Sqrt(5.f));

	for (int32 i = 0; i < RemainingCount; ++i)
	{
		const float t = (i + 0.5f) / RemainingCount;
		const float Radius = FMath::Sqrt(t);
		const float Theta = i * GoldenAngle;

		const float X = Radius * FMath::Cos(Theta);
		const float Y = Radius * FMath::Sin(Theta);

		const FVector Offset = (Right * X + Up * Y) * DiscRadius;
		const FVector BulletDir = (Forward + Offset).GetSafeNormal();

		Result.Add(BulletDir);
	}

	return Result;
}

float UGA_Fire::GetDamagePerBullet(const APDWeaponBase* Weapon) const
{
	if (!Weapon || !Weapon->WeaponData)
	{
		return 0.f;
	}

	return Weapon->WeaponData->WeaponDamage;
}

ETeamType UGA_Fire::GetActorTeam(AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return ETeamType::None;
	}

	if (Actor->GetClass()->ImplementsInterface(UPDTeamInterface::StaticClass()))
	{
		if (IPDTeamInterface* TeamActor = Cast<IPDTeamInterface>(Actor))
		{
			return TeamActor->GetTeamID();
		}
	}

	return ETeamType::None;
}

bool UGA_Fire::IsSameTeamActor(AActor* SourceActor, AActor* TargetActor) const
{
	const ETeamType SourceTeam = GetActorTeam(SourceActor);
	const ETeamType TargetTeam = GetActorTeam(TargetActor);

	if (SourceTeam == ETeamType::None || TargetTeam == ETeamType::None)
	{
		return false;
	}

	return SourceTeam == TargetTeam;
}

bool UGA_Fire::IsFriendlyShield(APDPawnBase* OwnerPawn, AActor* HitActor) const
{
	if (!IsValid(OwnerPawn) || !IsValid(HitActor))
	{
		return false;
	}

	const APDDamageableSkillActor* ShieldActor = Cast<APDDamageableSkillActor>(HitActor);
	if (!ShieldActor)
	{
		return false;
	}

	return IsSameTeamActor(OwnerPawn, HitActor);
}

EBulletHitDecision UGA_Fire::EvaluateHitDecision(APDPawnBase* OwnerPawn, AActor* HitActor) const
{
	if (!IsValid(OwnerPawn) || !IsValid(HitActor))
	{
		return EBulletHitDecision::Skip;
	}

	if (HitActor == OwnerPawn)
	{
		return EBulletHitDecision::Skip;
	}

	const bool bSameTeam = IsSameTeamActor(OwnerPawn, HitActor);

	if (const APDDamageableSkillActor* ShieldActor = Cast<APDDamageableSkillActor>(HitActor))
	{
		if (bSameTeam)
		{
			return EBulletHitDecision::Skip;
		}

		return EBulletHitDecision::BlockAndDamage;
	}

	if (Cast<APawn>(HitActor) || Cast<APDDestructibleObject>(HitActor))
	{
		if (bSameTeam)
		{
			return EBulletHitDecision::BlockOnly;
		}

		return EBulletHitDecision::BlockAndDamage;
	}

	if (HitActor->GetClass()->ImplementsInterface(UPDTeamInterface::StaticClass()))
	{
		if (bSameTeam)
		{
			return EBulletHitDecision::BlockOnly;
		}

		return EBulletHitDecision::BlockAndDamage;
	}

	return EBulletHitDecision::BlockOnly;
}

bool UGA_Fire::IsShieldActor(AActor* Actor) const
{
	return Cast<APDDamageableSkillActor>(Actor) != nullptr;
}
