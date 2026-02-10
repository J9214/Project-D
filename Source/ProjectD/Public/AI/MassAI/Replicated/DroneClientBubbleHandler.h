#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleHandler.h"
#include "MassCommonFragments.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"

class FDroneClientBubbleHandler : public TClientBubbleHandlerBase<FDroneFastArrayItem>
{
public:
	FDroneClientBubbleHandler();

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

#if UE_REPLICATION_COMPILE_SERVER_CODE
	FDroneFastArrayItem* GetMutableItem(const FMassReplicatedAgentHandle Handle);
	void MarkItemDirty(FDroneFastArrayItem& Item) const;
#endif

private:
	void ApplyReplicatedTransform(const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent) const;
};
