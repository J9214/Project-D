#pragma once

#include "CoreMinimal.h"
#include "MassSpawner.h"
#include "DroneSpawner.generated.h"

UCLASS()
class PROJECTD_API ADroneSpawner : public AMassSpawner
{
	GENERATED_BODY()

public:
	ADroneSpawner();

public:
	void SpawnDrones();

protected:
	void ProcessNextSpawnBatch();
	void ScheduleNextSpawnBatch();
	void FinishBatchSpawn();

protected:
	FORCEINLINE bool IsBatchSpawnActive() const { return RemainingBatchCount > 0; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone Spawn", meta = (ClampMin = "1"))
	int32 SpawnOverBatches = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone Spawn", meta = (ClampMin = "0.0"))
	float SpawnBatchInterval = 0.05f;

protected:
	FTimerHandle SpawnBatchTimerHandle;

	int32 InitialTotalSpawnCount = 0;
	int32 RemainingSpawnCount = 0;
	int32 RemainingBatchCount = 0;

	float OriginalSpawnScale = 1.0f;
};