#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleHandler.h"
#include "MassCommonFragments.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"

class UMassEntityEffectSubsystem;

class FDroneClientBubbleHandler : public TClientBubbleHandlerBase<FDroneFastArrayItem>
{
public:
	FDroneClientBubbleHandler();

	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

protected:
	virtual void InitializeForWorld(UWorld& World) override;
	virtual void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize) override;

#if UE_REPLICATION_COMPILE_SERVER_CODE
public:
	FDroneFastArrayItem* GetMutableItem(const FMassReplicatedAgentHandle Handle);
	void MarkItemDirty(FDroneFastArrayItem& Item) const;
	bool CleanAgent(const FMassReplicatedAgentHandle Handle);
#endif

private:
	void ApplyReplicatedTransform(const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent) const;

private:
	UMassEntityEffectSubsystem* EffectSubsystem = nullptr;
};
