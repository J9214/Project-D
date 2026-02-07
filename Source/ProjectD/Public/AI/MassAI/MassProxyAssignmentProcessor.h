// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "UObject/SoftObjectPtr.h"
#include "MassProxyAssignmentProcessor.generated.h"

class UMassProxyPoolSubsystem;
class UPlayerLocSubsystem;
class UMassPerceptionSubsystem;

struct FMassAimTubeRequest;
struct FTransformFragment;

/**
 *
 */
UCLASS()
class PROJECTD_API UMassProxyAssignmentProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassProxyAssignmentProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	float GetMinDistSqToPlayers(const FVector& Pos, const TArray<FVector>& PlayerLocs);

	bool IsNearAnyPlayer(const FVector& Pos, const TArray<FVector>& PlayerLocs, float NearRadiusSq);

	bool IsInAnySightCone(
		const FVector& Pos,
		const TArray<FVector>& ViewOrigins,
		const TArray<FVector>& ViewDirs,
		float SightMaxDistSq,
		float SightCosHalfAngle
	);

	bool IsPointInAimTube(const FVector& Point, const FMassAimTubeRequest& Tube, float TubeRadius);
	void ApplyMaxDesiredEntities(int32 MaxDesiredEntities, TArray<FMassEntityHandle>& InOutDesired);
	void ReleaseAllAssigned();

private:
	FMassEntityQuery EntityQuery;

	float TimeSinceLastUpdateSeconds = 0.0f;

	UMassProxyPoolSubsystem* ProxyPoolSubsystem = nullptr;
	UPlayerLocSubsystem* PlayerLocSubsystem = nullptr;
	UMassPerceptionSubsystem* PerceptionSubsystem = nullptr;

	TSet<FMassEntityHandle> CurrentlyAssignedEntities;
};
