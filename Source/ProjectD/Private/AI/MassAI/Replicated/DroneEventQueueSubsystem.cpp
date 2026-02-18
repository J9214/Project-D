#include "AI/MassAI/Replicated/DroneEventQueueSubsystem.h"

void UDroneEventQueueSubsystem::EnqueueDeathEvent(const FMassNetworkID NetID, const FVector& DeathLocation, const EMassEntityCueId CueId)
{
	FDroneDeathEvent E;
	E.NetID = NetID;
	E.DeathLocation = DeathLocation;
	E.CueId = CueId;

	DeathQueue.Add(E);

	// Delete After One Frame
	E.RemoveAfterFrame = static_cast<uint64>(GFrameCounter) + 1;
	RemoveQueue.Add(E);
}

void UDroneEventQueueSubsystem::MoveDeathArray(TArray<FDroneDeathEvent>& OutDeaths)
{
	OutDeaths = MoveTemp(DeathQueue);
	DeathQueue.Reset();
}

void UDroneEventQueueSubsystem::MoveDueRemovals(TArray<FMassNetworkID>& OutRemovals)
{
	const uint64 NowFrame = static_cast<uint64>(GFrameCounter);

	for (int32 i = RemoveQueue.Num() - 1; i >= 0; --i)
	{
		if (RemoveQueue[i].RemoveAfterFrame <= NowFrame)
		{
			OutRemovals.Add(RemoveQueue[i].NetID);
			RemoveQueue.RemoveAtSwap(i);
		}
	}
}