#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "AI/MassAI/MassEntityCueId.h"
#include "DeathScheduleFragment.generated.h"

USTRUCT()
struct PROJECTD_API FDeathScheduleFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	uint32 RemoveAtFrame = 0;
	uint32 DestroyAtFrame = 0;

	FVector_NetQuantize DeathLoc = FVector::ZeroVector;

	bool bMarkedForRemoval = false;
};