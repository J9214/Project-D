// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset_MassPerceptionSettings.generated.h"

USTRUCT(BlueprintType)
struct PROJECTD_API FMassPerceptionTuning
{
	GENERATED_BODY()

	// Near Player Radius
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Near")
	float NearRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Sight")
	float SightHalfAngleDeg = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Sight")
	float SightMaxDist = 3500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Aim")
	float AimTubeRadius = 150.f;

	// AimTubeTimeToLive
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Aim")
	float AimTubeTTL = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Perf")
	float UpdateInterval = 0.05f;

	// Max Limits (For Proxy Pool)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Perf")
	int32 MaxDesiredEntities = 300;

	// Max Request Limits (For MassPerceptionType)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception|Perf")
	int32 MaxDesiredRequests = 128;
};

UCLASS(BlueprintType)
class PROJECTD_API UDataAsset_MassPerceptionSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception")
	FMassPerceptionTuning Tuning;

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("MassPerception", GetFName());
	}
};
