#include "AI/MassAI/DroneSpawner.h"
#include "TimerManager.h"
#include "ProjectD/ProjectD.h"

ADroneSpawner::ADroneSpawner()
{
	bReplicates = false;
	SetReplicateMovement(false);
	bAlwaysRelevant = false;
	bNetLoadOnClient = false;
}

void ADroneSpawner::SpawnDrones()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[DroneSpawner] SpawnDrones skipped. Not authority."));
		return;
	}

	if (IsBatchSpawnActive() == true)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[DroneSpawner] SpawnDrones skipped. Batch spawn already active."));
		return;
	}

	OriginalSpawnScale = GetSpawningCountScale();
	InitialTotalSpawnCount = GetSpawnCount();

	if (InitialTotalSpawnCount <= 0)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[DroneSpawner] SpawnDrones failed. InitialTotalSpawnCount <= 0."));
		return;
	}

	RemainingSpawnCount = InitialTotalSpawnCount;
	RemainingBatchCount = FMath::Max(1, SpawnOverBatches);

	UE_LOG(
		LogProjectD,
		Log,
		TEXT("[DroneSpawner] SpawnDrones started. InitialTotalSpawnCount=%d, SpawnOverBatches=%d, SpawnBatchInterval=%.3f, OriginalSpawnScale=%.3f"),
		InitialTotalSpawnCount,
		RemainingBatchCount,
		SpawnBatchInterval,
		OriginalSpawnScale
	);

	ProcessNextSpawnBatch();
}

void ADroneSpawner::ProcessNextSpawnBatch()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[DroneSpawner] ProcessNextSpawnBatch skipped. Not authority."));
		FinishBatchSpawn();
		return;
	}

	if (RemainingBatchCount <= 0)
	{
		FinishBatchSpawn();
		return;
	}

	if (RemainingSpawnCount <= 0)
	{
		FinishBatchSpawn();
		return;
	}

	if (InitialTotalSpawnCount <= 0)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[DroneSpawner] ProcessNextSpawnBatch failed. InitialTotalSpawnCount <= 0."));
		FinishBatchSpawn();
		return;
	}

	const int32 ThisBatchCount = FMath::CeilToInt(static_cast<float>(RemainingSpawnCount) / static_cast<float>(RemainingBatchCount));
	const float BatchScale = OriginalSpawnScale * (static_cast<float>(ThisBatchCount) / static_cast<float>(InitialTotalSpawnCount));

	ScaleSpawningCount(BatchScale);

	UE_LOG(
		LogProjectD,
		Log,
		TEXT("[DroneSpawner] ProcessNextSpawnBatch. ThisBatchCount=%d, RemainingSpawnCount=%d, RemainingBatchCount=%d, BatchScale=%.4f"),
		ThisBatchCount,
		RemainingSpawnCount,
		RemainingBatchCount,
		BatchScale
	);

	DoSpawning();

	RemainingSpawnCount -= ThisBatchCount;
	RemainingBatchCount -= 1;

	if (RemainingSpawnCount > 0 && RemainingBatchCount > 0)
	{
		ScheduleNextSpawnBatch();
	}
	else
	{
		FinishBatchSpawn();
	}
}

void ADroneSpawner::ScheduleNextSpawnBatch()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[DroneSpawner] ScheduleNextSpawnBatch failed. World is invalid."));
		FinishBatchSpawn();
		return;
	}

	if (SpawnBatchInterval <= KINDA_SMALL_NUMBER)
	{
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &ADroneSpawner::ProcessNextSpawnBatch);
		World->GetTimerManager().SetTimerForNextTick(TimerDelegate);
		return;
	}

	World->GetTimerManager().SetTimer(
		SpawnBatchTimerHandle,
		this,
		&ADroneSpawner::ProcessNextSpawnBatch,
		SpawnBatchInterval,
		false
	);
}

void ADroneSpawner::FinishBatchSpawn()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == true)
	{
		World->GetTimerManager().ClearTimer(SpawnBatchTimerHandle);
	}

	ScaleSpawningCount(OriginalSpawnScale);

	UE_LOG(
		LogProjectD,
		Log,
		TEXT("[DroneSpawner] FinishBatchSpawn. RemainingSpawnCount=%d, RemainingBatchCount=%d, RestoreScale=%.3f"),
		RemainingSpawnCount,
		RemainingBatchCount,
		OriginalSpawnScale
	);

	InitialTotalSpawnCount = 0;
	RemainingSpawnCount = 0;
	RemainingBatchCount = 0;
	OriginalSpawnScale = 1.0f;
}