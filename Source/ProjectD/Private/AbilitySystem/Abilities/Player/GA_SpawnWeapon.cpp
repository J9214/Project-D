#include "AbilitySystem/Abilities/Player/GA_SpawnWeapon.h"
#include "Weapon/PDWeaponBase.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Pawn/PDPawnBase.h"

UGA_SpawnWeapon::UGA_SpawnWeapon()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
	ActivationPolicy = EPDAbilityActivationPolicy::OnGiven;
}

void UGA_SpawnWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!WeaponClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (APDPawnBase* Pawn = GetPlayerPawnFromActorInfo())
	{
		if (UWeaponManageComponent* WMC = Pawn->GetWeaponManageComponent())
		{
			WMC->Server_BuyWeapon(WeaponClass);
		}
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
