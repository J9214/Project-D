#include "AbilitySystem/Abilities/Player/GA_Fire.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Pawn/PDPawnBase.h"
#include "Weapon/PDWeaponBase.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "PDGameplayTags.h"

UGA_Fire::UGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
	ActivationPolicy = EPDAbilityActivationPolicy::WhileInputActive;
}

void UGA_Fire::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(ActorInfo->AvatarActor.Get());
	if (!OwnerPawn)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UWeaponManageComponent* WMC = GetWeaponManageComponentFromActorInfo();
	if (!WMC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	APDWeaponBase* Weapon = WMC->GetEquippedWeapon();
	if (!IsValid(Weapon) || !Weapon->WeaponData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (IsLocallyControlled())
	{
		if (!Weapon->ClientCanFire())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
	
	const UDataAsset_Weapon* WeaponDA = Weapon->WeaponData;
	const FPDWeaponMontageEntry& Entry = WeaponDA->WeaponMontages.Get(EPDWeaponMontageAction::Fire);
	
	UAnimMontage* MontageToPlay = Entry.Montage;
	bool bStopWhenAbilityEnds = Entry.bStopWhenAbilityEnds;
	
	UAbilityTask_PlayMontageAndWait* PlayTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("FireMontageTask"), MontageToPlay, 1.f, NAME_None, bStopWhenAbilityEnds);
	if (IsValid(PlayTask))
	{
		PlayTask->OnCompleted.AddDynamic(this, &UGA_Fire::OnMontageCompleted);
		PlayTask->OnInterrupted.AddDynamic(this, &UGA_Fire::OnMontageInterrupted);
		PlayTask->OnCancelled.AddDynamic(this, &UGA_Fire::OnMontageCancelled);
		PlayTask->ReadyForActivation();
	}

	const FPredictionKey PredKey = ActivationInfo.GetActivationPredictionKey();

	if (HasAuthority(&ActivationInfo))
	{
		ASC->AbilityTargetDataSetDelegate(Handle, PredKey)
			.AddUObject(this, &UGA_Fire::OnTargetDataReceived);

		ASC->CallAllReplicatedDelegatesIfSet(Handle, PredKey);
		
		return;
	}
	
	if (ActorInfo->IsLocallyControlled())
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		const float MaxRange = Weapon->WeaponData->MaxRange;

		const FVector ViewLocation = OwnerPawn->GetPawnViewLocation();
		FRotator ViewRotation = OwnerPawn->GetBaseAimRotation();
		if (AController* Controller = OwnerPawn->GetController())
		{
			ViewRotation = Controller->GetControlRotation();
		}

		const FVector AimDirection = ViewRotation.Vector();
		const FVector CameraStart = ViewLocation;
		const FVector CameraEnd = CameraStart + AimDirection * MaxRange;

		FCollisionQueryParams Params(SCENE_QUERY_STAT(GA_Fire_CameraTrace_Client), false, OwnerPawn);
		Params.AddIgnoredActor(Weapon);

		FHitResult CameraHit;
		const bool bHit = World->LineTraceSingleByChannel(
			CameraHit,
			CameraStart,
			CameraEnd,
			ECC_GameTraceChannel1,
			Params
		);

		const FVector AimPoint = bHit ? CameraHit.ImpactPoint : CameraEnd;
		const FGameplayAbilityTargetDataHandle TargetData = MakeAimPointTargetData(CameraStart, AimPoint);

		FScopedPredictionWindow PredWindow(ASC, true);
		ASC->CallServerSetReplicatedTargetData(
			Handle,
			PredKey,
			TargetData,
			FGameplayTag(),
			ASC->ScopedPredictionKey
		);
		
		DrawDebugLine(World, CameraStart, AimPoint, FColor::Cyan, false, 1.f, 0, 1.f);
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_Fire::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
        return;
    }
 
    APDPawnBase* OwnerPawn = Cast<APDPawnBase>(ActorInfo->AvatarActor.Get());
    UWeaponManageComponent* WMC = OwnerPawn ? OwnerPawn->GetWeaponManageComponent() : nullptr;
    APDWeaponBase* Weapon = WMC ? WMC->GetEquippedWeapon() : nullptr;
    if (!OwnerPawn || !WMC || !IsValid(Weapon) || !Weapon->WeaponData)
    {
        EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
        return;
    }
	
	const FGameplayAbilityTargetData* Raw = Data.Get(0);
	if (!Raw || Raw->GetScriptStruct() != FGameplayAbilityTargetData_LocationInfo::StaticStruct())
	{
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
    const FGameplayAbilityTargetData_LocationInfo* LocInfo = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(Raw);
    if (!LocInfo)
    {
        EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
        return;
    }
	
    ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
	
	if (!Weapon->ServerCanFire())
	{
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
		return;
	}
 
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	Weapon->ServerConsumeAmmo(1);
	ApplyFireCooldownToOwner(Weapon);
	
    const FVector AimPoint = LocInfo->TargetLocation.GetTargetingTransform().GetLocation();
	MuzzleTraceAndApplyGE(OwnerPawn, Weapon, AimPoint);
	
    EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Fire::MuzzleTraceAndApplyGE(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon, const FVector& AimPoint)
{
	UWorld* World = GetWorld();
	const float MaxRange = Weapon->WeaponData->MaxRange;

	const FVector MuzzleStart = Weapon->GetMuzzlePoint();
	FVector FireDir = (AimPoint - MuzzleStart).GetSafeNormal();
	if (FireDir.IsNearlyZero())
	{
		FireDir = OwnerPawn->GetActorForwardVector();
	}

	const FVector MuzzleEnd = MuzzleStart + FireDir * MaxRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GA_Fire_MuzzleTrace_Server), false, OwnerPawn);
	Params.AddIgnoredActor(Weapon);

	FHitResult Hit;
	const bool bHit = World && World->LineTraceSingleByChannel(
		Hit,
		MuzzleStart,
		MuzzleEnd,
		ECC_GameTraceChannel1,
		Params
	);

	ApplyWeaponDamageGE(Hit, Weapon);
	
	OwnerPawn->ClientDrawFireDebug(MuzzleStart, bHit? Hit.ImpactPoint : MuzzleEnd, bHit, Hit.ImpactPoint);
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

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Weapon->WeaponData->WeaponDamageGE, Level, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_Weapon_Damage, Damage);
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void UGA_Fire::ApplyFireCooldownToOwner(const APDWeaponBase* Weapon)
{
	if (!Weapon || !Weapon->WeaponData || !Weapon->WeaponData->FireCooldownGE)
	{
		return;
	}

	const float Interval = FMath::Max(0.01f, Weapon->WeaponData->FireInterval);

	FGameplayEffectSpecHandle CooldownEffect = MakeOutgoingGameplayEffectSpec(Weapon->WeaponData->FireCooldownGE, GetAbilityLevel());
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
