#pragma once

#include "CoreMinimal.h"
#include "PDWeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EPDWeaponTypes : uint8
{
	None,
	Pistol,
	Rifle,
	Shotgun,
	Throwable
};

UENUM(BlueprintType)
enum class EPDWeaponFireMode : uint8
{
	SemiAuto,
	FullAuto
};