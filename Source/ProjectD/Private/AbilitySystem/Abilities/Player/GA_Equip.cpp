#include "AbilitySystem/Abilities/Player/GA_Equip.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Weapon/PDWeaponMontages.h"

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
	UWeaponManageComponent* WMC = GetWeaponManageComponentFromActorInfo();
	if (!WMC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	FPDWeaponMontageEntry Entry;
	if (!WMC->TryGetMontageEntry(EquipSlotIndex, EPDWeaponMontageAction::Equip, Entry))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	int32 CurrentEquippedSlotIndex = WMC->GetEquippedSlotIndex();
	if (CurrentEquippedSlotIndex == EquipSlotIndex || !WMC->HasItemInSlot(EquipSlotIndex))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	if (HasAuthority(&ActivationInfo))
	{
		if (Entry.CommitEventTag.IsValid() && Entry.Montage)
		{
			auto* WaitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this,
				Entry.CommitEventTag,
				nullptr,
				true,
				false
			);
			WaitEvent->EventReceived.AddDynamic(this, &UGA_Equip::OnEventTagReceived);
			WaitEvent->ReadyForActivation();
		}
		else
		{
			if (CommitAbility(Handle, ActorInfo, ActivationInfo))
			{
				WMC->EquipSlot(EquipSlotIndex);
			}
			
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
	
	if (Entry.Montage)
	{
		UAbilityTask_PlayMontageAndWait* PlayTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, TEXT("EquipMontageTask"), Entry.Montage, Entry.PlayRate);
		if (IsValid(PlayTask))
		{
			PlayTask->OnCompleted.AddDynamic(this, &UGA_Equip::OnMontageCompleted);
			PlayTask->OnInterrupted.AddDynamic(this, &UGA_Equip::OnMontageInterrupted);
			PlayTask->OnCancelled.AddDynamic(this, &UGA_Equip::OnMontageCancelled);
			PlayTask->ReadyForActivation();
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_Equip::OnEventTagReceived(const FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	UWeaponManageComponent* WMC = GetWeaponManageComponentFromActorInfo();
	if (!WMC || !WMC->HasItemInSlot(EquipSlotIndex))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	WMC->EquipSlot(EquipSlotIndex);
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
