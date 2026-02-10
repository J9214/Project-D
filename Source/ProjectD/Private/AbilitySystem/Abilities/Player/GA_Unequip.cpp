#include "AbilitySystem/Abilities/Player/GA_Unequip.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Weapon/PDWeaponBase.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"

UGA_Unequip::UGA_Unequip()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_Unequip::ActivateAbility(
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
	const FPDWeaponMontageEntry& Entry = WeaponDA->WeaponMontages.Get(EPDWeaponMontageAction::Unequip);
	
	UAnimMontage* MontageToPlay = Entry.Montage;
	FGameplayTag CommitTag = Entry.CommitEventTag;
	float PlayRate = Entry.PlayRate;
	
	if (HasAuthority(&ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* WaitEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, CommitTag, nullptr, false, false);
		
		WaitEventTask->EventReceived.AddDynamic(this, &UGA_Unequip::OnEventTagReceived);
		WaitEventTask->ReadyForActivation();
	}
	
	UAbilityTask_PlayMontageAndWait* PlayTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("UnequipMontageTask"), MontageToPlay, PlayRate);
	if (IsValid(PlayTask))
	{
		PlayTask->OnCompleted.AddDynamic(this, &UGA_Unequip::OnMontageCompleted);
		PlayTask->OnInterrupted.AddDynamic(this, &UGA_Unequip::OnMontageInterrupted);
		PlayTask->OnCancelled.AddDynamic(this, &UGA_Unequip::OnMontageCancelled);
		PlayTask->ReadyForActivation();
	}
}

void UGA_Unequip::OnEventTagReceived(const FGameplayEventData Payload)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!Avatar->HasAuthority())
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
	
	WMC->UnequipCurrentWeapon();
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
