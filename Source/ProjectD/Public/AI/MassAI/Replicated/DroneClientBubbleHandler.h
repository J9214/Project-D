#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleHandler.h"
#include "MassCommonFragments.h"
#include "AI/MassAI/Replicated/ClientVisualFragment.h"
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

#if !UE_SERVER
	virtual void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize) override;
#endif

#if UE_REPLICATION_COMPILE_SERVER_CODE
public:
	FDroneFastArrayItem* GetMutableItem(const FMassReplicatedAgentHandle Handle);
	void MarkItemDirty(FDroneFastArrayItem& Item) const;
	bool CleanAgent(const FMassReplicatedAgentHandle Handle);
#endif

private:
	void ApplyReplicatedVisualState(const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent, const bool bForceSnap) const;
	void TryPlayDeathCueOnce(const FDroneReplicatedAgent& ReplicatedAgent);

private:
	UMassEntityEffectSubsystem* EffectSubsystem = nullptr;

	// For Death FX
	TSet<uint32> PlayedDeathFxNetIDs;
};
