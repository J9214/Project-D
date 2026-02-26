#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "ClientVisualFragment.generated.h"

USTRUCT()
struct FClientVisualFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	FVector TargetLocation = FVector::ZeroVector;
	FQuat TargetRotation = FQuat::Identity;

	FVector DeathLocation = FVector::ZeroVector;

	bool bDead = false;
	bool bInitialized = false;
	bool bSnapThisFrame = false;
};