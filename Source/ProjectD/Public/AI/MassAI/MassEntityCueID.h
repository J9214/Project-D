#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMassEntityCueId : uint8
{
	None = 0,

	// Drone
	Drone_Death = 1,
	Drone_Explode = 2,
};