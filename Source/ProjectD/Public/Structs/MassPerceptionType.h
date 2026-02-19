#pragma once

#include "CoreMinimal.h"

struct FMassAimTubeRequest
{
	FVector Origin = FVector::ZeroVector;
	FVector Dir = FVector::ForwardVector;
	float WeaponLength = 0.f;
	float TubeRadius = 0.f;
	float ExpireTime = 0.f;

	FORCEINLINE bool IsExpired(float Now) const { return Now >= ExpireTime; }
};
