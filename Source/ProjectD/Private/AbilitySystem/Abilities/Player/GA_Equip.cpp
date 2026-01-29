#include "AbilitySystem/Abilities/Player/GA_Equip.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Pawn/PDPawnBase.h"
#include "Weapon/PDWeaponBase.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"

UGA_Equip::UGA_Equip()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_Equip::ActivateAbility(
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
	
	APDWeaponBase* Weapon = WMC->GetWeaponInSlot(EquipSlotIndex);
	if (!IsValid(Weapon) || !Weapon->WeaponData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	APDWeaponBase* CurrentEquippedWeapon = WMC->GetEquippedWeapon();
	if (CurrentEquippedWeapon == Weapon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const UDataAsset_Weapon* WeaponDA = Weapon->WeaponData;
	const FPDWeaponMontageEntry& Entry = WeaponDA->WeaponMontages.Get(EPDWeaponMontageAction::Equip);
	
	UAnimMontage* MontageToPlay = Entry.Montage;
	FGameplayTag CommitTag = Entry.CommitEventTag;
	float PlayRate = Entry.PlayRate;
	
	if (HasAuthority(&ActivationInfo))
	{
		UAbilityTask_WaitGameplayEvent* WaitEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, CommitTag, nullptr, false, false);
		
		WaitEventTask->EventReceived.AddDynamic(this, &UGA_Equip::OnEventTagReceived);
		WaitEventTask->ReadyForActivation();
	}
	
	UAbilityTask_PlayMontageAndWait* PlayTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("EquipMontageTask"), MontageToPlay, PlayRate);
	if (IsValid(PlayTask))
	{
		PlayTask->OnCompleted.AddDynamic(this, &UGA_Equip::OnMontageCompleted);
		PlayTask->OnInterrupted.AddDynamic(this, &UGA_Equip::OnMontageInterrupted);
		PlayTask->OnCancelled.AddDynamic(this, &UGA_Equip::OnMontageCancelled);
		PlayTask->ReadyForActivation();
	}
}

void UGA_Equip::OnEventTagReceived(const FGameplayEventData Payload)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	if (!Avatar->HasAuthority())
	{
		return;
	}
	
	UWeaponManageComponent* WMC = GetWeaponManageComponentFromActorInfo();
	if (!WMC || !WMC->GetWeaponInSlot(EquipSlotIndex))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	WMC->EquipSlot(EquipSlotIndex);
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
