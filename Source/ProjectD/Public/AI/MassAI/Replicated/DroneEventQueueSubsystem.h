// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassReplicationTypes.h"
#include "AI/MassAI/MassEntityCueId.h"
#include "DroneEventQueueSubsystem.generated.h"

USTRUCT()
struct FDroneDeathEvent
{
	GENERATED_BODY()

	UPROPERTY()
	FMassNetworkID NetID;

	UPROPERTY()
	FVector_NetQuantize DeathLocation = FVector::ZeroVector;

	UPROPERTY()
	EMassEntityCueId CueId = EMassEntityCueId::Drone_Death;

	UPROPERTY()
	uint64 RemoveAfterFrame = 0;
};

UCLASS()
class PROJECTD_API UDroneEventQueueSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void EnqueueDeathEvent(const FMassNetworkID NetID, const FVector& DeathLocation, const EMassEntityCueId CueId);
	void MoveDeathArray(TArray<FDroneDeathEvent>& OutDeaths);
	void MoveDueRemovals(TArray<FMassNetworkID>& OutRemovals);

private:
	UPROPERTY()
	TArray<FDroneDeathEvent> DeathQueue;

	UPROPERTY()
	TArray<FDroneDeathEvent> RemoveQueue;
};
