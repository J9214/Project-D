#include "Weapon/PDWeaponBase.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Controller/PDPlayerController.h"

APDWeaponBase::APDWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);
	
	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	Muzzle->SetupAttachment(RootComp);
}

void APDWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APDWeaponBase, CurrentFireMode);
	DOREPLIFETIME(APDWeaponBase, MaxAmmo);
	DOREPLIFETIME(APDWeaponBase, CurrentAmmo);
}

void APDWeaponBase::Server_SetFireMode_Implementation(EPDWeaponFireMode NewMode)
{
	CurrentFireMode = NewMode;
}

FVector APDWeaponBase::GetMuzzlePoint() const
{
	return Muzzle ? Muzzle->GetComponentLocation() : FVector::ZeroVector;
}

bool APDWeaponBase::CanFire() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	
	if (CurrentAmmo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Ammo"));
		return false;
	}
	
	return true;
}

void APDWeaponBase::ConsumeAmmo(int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}

	Amount = FMath::Max(1, Amount);

	if (CurrentAmmo < Amount)
	{
		return;
	}

	CurrentAmmo -= Amount;
	CurrentAmmo = FMath::Clamp(CurrentAmmo, 0, MaxAmmo);
}

void APDWeaponBase::InitWeaponData()
{
	if (WeaponData)
	{
		CurrentFireMode = WeaponData->DefaultFireMode;
		MaxAmmo = WeaponData->MaxAmmo;
		CurrentAmmo = MaxAmmo;
	}
}

void APDWeaponBase::ReloadAmmo()
{
	CurrentAmmo = MaxAmmo;
}

void APDWeaponBase::OnRep_AmmoChanged()
{
	// Update HUD
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn || !OwningPawn->IsLocallyControlled())
	{
		return;
	}

	APDPlayerController* PC = Cast<APDPlayerController>(OwningPawn->GetController());
	if (!PC)
	{
		return;
	}

	PC->UpdateCurrentAmmo(CurrentAmmo);
}

void APDWeaponBase::OnRep_FireModeChanged()
{
	// Update HUD, Sound
}

EPDWeaponFireMode APDWeaponBase::GetNextFireMode() const
{
	if (!WeaponData)
	{
		return CurrentFireMode;
	}

	const TArray<EPDWeaponFireMode>& Modes = WeaponData->SupportedFireModes;
	if (Modes.Num() == 0)
	{
		return CurrentFireMode;
	}

	int32 Index = Modes.IndexOfByKey(CurrentFireMode);
	if (Index == INDEX_NONE)
	{
		if (Modes.Contains(WeaponData->DefaultFireMode))
		{
			return WeaponData->DefaultFireMode;
		}
		
		return Modes[0];
	}

	Index = (Index + 1) % Modes.Num();
	
	return Modes[Index];
}

FTransform APDWeaponBase::GetSightCameraWorldTransform() const
{
	TArray<USceneComponent*> SceneComps;
	GetComponents<USceneComponent>(SceneComps);

	for (USceneComponent* Comp : SceneComps)
	{
		if (!Comp)
		{
			continue;
		}
		
		if (Comp->GetFName() == SightAnchorComponentName)
		{
			return Comp->GetComponentTransform();
		}
	}

	return GetActorTransform();
}

bool APDWeaponBase::IsMultiBulletWeapon() const
{
	return WeaponData && WeaponData->SpreadConfig.BulletsPerShot > 1;
}

int32 APDWeaponBase::GetBulletsPerShot() const
{
	if (!WeaponData)
	{
		return 1;
	}

	return FMath::Max(1, WeaponData->SpreadConfig.BulletsPerShot);
}

float APDWeaponBase::GetSpreadHalfAngleDeg() const
{
	if (!WeaponData)
	{
		return 0.f;
	}

	return FMath::Max(0.f, WeaponData->SpreadConfig.SpreadHalfAngleDeg);
}