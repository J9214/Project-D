#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "DronePendingExplosionFragment.generated.h"

USTRUCT()
struct PROJECTD_API FDronePendingExplosionFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	int32 RemainingFrames = 0;
};