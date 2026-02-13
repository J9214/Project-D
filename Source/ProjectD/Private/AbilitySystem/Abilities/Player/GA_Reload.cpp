#include "AbilitySystem/Abilities/Player/GA_Reload.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Weapon/PDWeaponBase.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"

UGA_Reload::UGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_Reload::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
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
	
	const UDataAsset_Weapon* WeaponDA = Weapon->WeaponData;
	const FPDWeaponMontageEntry& Entry = WeaponDA->WeaponMontages.Get(EPDWeaponMontageAction::Reload);
	
	UAnimMontage* MontageToPlay = Entry.Montage;
	FGameplayTag CommitTag = Entry.CommitEventTag;
	float PlayRate = Entry.PlayRate;
	
	if (HasAuthority(&ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* WaitEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, CommitTag, nullptr, false, false);
		
		WaitEventTask->EventReceived.AddDynamic(this, &UGA_Reload::OnEventTagReceived);
		WaitEventTask->ReadyForActivation();
	}
	
	UAbilityTask_PlayMontageAndWait* PlayTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("ReloadMontageTask"), MontageToPlay, PlayRate);
	if (IsValid(PlayTask))
	{
		PlayTask->OnCompleted.AddDynamic(this, &UGA_Reload::OnMontageCompleted);
		PlayTask->OnInterrupted.AddDynamic(this, &UGA_Reload::OnMontageInterrupted);
		PlayTask->OnCancelled.AddDynamic(this, &UGA_Reload::OnMontageCancelled);
		PlayTask->ReadyForActivation();
	}
}

void UGA_Reload::OnEventTagReceived(const FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	UWeaponManageComponent* WMC = GetWeaponManageComponentFromActorInfo();
	if (!WMC)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	APDWeaponBase* Weapon = WMC->GetEquippedWeapon();
	if (!Weapon)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	Weapon->ReloadAmmo();
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
