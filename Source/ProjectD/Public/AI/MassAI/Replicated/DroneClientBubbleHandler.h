#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleHandler.h"
#include "MassCommonFragments.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"

class UMassEntitySubsystem;
class UMassReplicationSubsystem;

class FDroneClientBubbleHandler : public TClientBubbleHandlerBase<FDroneFastArrayItem>
{
public:
	FDroneClientBubbleHandler();

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

#if UE_REPLICATION_COMPILE_SERVER_CODE
public:
	FDroneFastArrayItem* GetMutableItem(const FMassReplicatedAgentHandle Handle);
	void MarkItemDirty(FDroneFastArrayItem& Item) const;
	
	void RegisterNetIdHandle(const FMassNetworkID NetID, const FMassReplicatedAgentHandle Handle);
	bool MarkDeadByNetId(const FMassNetworkID NetID, const FVector_NetQuantize& DeathLoc, const uint8 CueId);
	bool RemoveByNetId(const FMassNetworkID NetID);
	
	bool CleanAgent(const FMassReplicatedAgentHandle Handle);

private:
	TMap<uint32, FMassReplicatedAgentHandle> NetIdToHandle;
#endif

private:
	void ApplyReplicatedTransform(const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent) const;
};
