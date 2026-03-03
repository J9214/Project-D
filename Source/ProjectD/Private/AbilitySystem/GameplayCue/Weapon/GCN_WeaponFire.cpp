#include "AbilitySystem/GameplayCue/Weapon/GCN_WeaponFire.h"
#include "Weapon/PDWeaponBase.h"
#include "Pawn/PDPawnBase.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

bool UGCN_WeaponFire::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	const APDPawnBase* OwnerPawn = Cast<APDPawnBase>(Parameters.Instigator.Get());
	if (!OwnerPawn)
	{
		return false;
	}
	
	if (OwnerPawn->IsLocallyControlled() || OwnerPawn->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}
	
	const APDWeaponBase* Weapon = Cast<APDWeaponBase>(Parameters.EffectCauser.Get());
	if (!Weapon)
	{
		return false;
	}
	
	const UDataAsset_Weapon* WeaponDA = Weapon->WeaponData;
	if (!WeaponDA)
	{
		return false;
	}
	
	USceneComponent* MuzzleComp = Weapon->GetMuzzleComponent();
	if (!MuzzleComp)
	{
		return false;
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
	
	return true;
}
