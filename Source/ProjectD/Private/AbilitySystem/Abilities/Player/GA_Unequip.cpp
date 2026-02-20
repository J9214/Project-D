#include "AbilitySystem/Abilities/Player/GA_Unequip.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
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
	UWeaponManageComponent* WMC = GetWeaponManageComponentFromActorInfo();
	if (!WMC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	int32 CurrentEquippedSlotIndex = WMC->GetEquippedSlotIndex();
	FPDWeaponMontageEntry Entry;
	if (!WMC->TryGetEquipEntry(CurrentEquippedSlotIndex, EPDWeaponMontageAction::Unequip, Entry))
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
			WaitEvent->EventReceived.AddDynamic(this, &UGA_Unequip::OnEventTagReceived);
			WaitEvent->ReadyForActivation();
		}
		else
		{
			if (CommitAbility(Handle, ActorInfo, ActivationInfo))
			{
				WMC->UnequipCurrentWeapon();
			}
			
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
	
	if (Entry.Montage)
	{
		UAbilityTask_PlayMontageAndWait* PlayTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, TEXT("UnequipMontageTask"), Entry.Montage, Entry.PlayRate);
		if (IsValid(PlayTask))
		{
			PlayTask->OnCompleted.AddDynamic(this, &UGA_Unequip::OnMontageCompleted);
			PlayTask->OnInterrupted.AddDynamic(this, &UGA_Unequip::OnMontageInterrupted);
			PlayTask->OnCancelled.AddDynamic(this, &UGA_Unequip::OnMontageCancelled);
			PlayTask->ReadyForActivation();
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_Unequip::OnEventTagReceived(const FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	UWeaponManageComponent* WMC = GetWeaponManageComponentFromActorInfo();
	if (!WMC)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	WMC->UnequipCurrentWeapon();
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
