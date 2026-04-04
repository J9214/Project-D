#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "Controller/PDPlayerController.h"
#include "Pawn/PDPawnBase.h"
#include "Components/Combat/WeaponManageComponent.h"

APDPawnBase* UPDPlayerGameplayAbility::GetPlayerPawnFromActorInfo()
{
	if (!CachedPlayerPawn.IsValid())
	{
		CachedPlayerPawn = Cast<APDPawnBase>(GetAvatarActorFromActorInfo());
	}
	
	return CachedPlayerPawn.IsValid() ? CachedPlayerPawn.Get() : nullptr;
}

APDPlayerController* UPDPlayerGameplayAbility::GetPlayerControllerFromActorInfo()
{
	if (!CachedPlayerController.IsValid())
	{
		if (APDPawnBase* Pawn = GetPlayerPawnFromActorInfo())
		{
			CachedPlayerController = Cast<APDPlayerController>(Pawn->GetController());
		}
	}
	
	return CachedPlayerController.IsValid() ? CachedPlayerController.Get() : nullptr;
}

UWeaponManageComponent* UPDPlayerGameplayAbility::GetWeaponManageComponentFromActorInfo()
{
	if (APDPawnBase* Pawn = GetPlayerPawnFromActorInfo())
	{
		return Pawn->GetWeaponManageComponent();
	}
	
	return nullptr;
}
