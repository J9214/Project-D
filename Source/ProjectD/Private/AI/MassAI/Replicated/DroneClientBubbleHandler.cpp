#include "AI/MassAI/Replicated/DroneClientBubbleHandler.h"

FDroneClientBubbleHandler::FDroneClientBubbleHandler()
	: TClientBubbleHandlerBase<FDroneFastArrayItem>()
{
}

void FDroneClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	auto AddRequirementsForSpawnQuery = [](FMassEntityQuery& Query)
		{
			Query.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
		};

	auto CacheFragmentViewsForSpawnQuery = [](FMassExecutionContext& Context)
		{
		};

	auto SetSpawnedEntityData = [this](const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent, const int32 EntityIdx)
		{
			ApplyReplicatedTransform(EntityView, ReplicatedAgent);
		};

	auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent)
		{
			ApplyReplicatedTransform(EntityView, ReplicatedAgent);
		};

	PostReplicatedAddHelper(
		AddedIndices,
		AddRequirementsForSpawnQuery,
		CacheFragmentViewsForSpawnQuery,
		SetSpawnedEntityData,
		SetModifiedEntityData
	);
}

void FDroneClientBubbleHandler::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent)
		{
			ApplyReplicatedTransform(EntityView, ReplicatedAgent);
		};

	PostReplicatedChangeHelper(
		ChangedIndices,
		SetModifiedEntityData
	);
}

#if UE_REPLICATION_COMPILE_SERVER_CODE
FDroneFastArrayItem* FDroneClientBubbleHandler::GetMutableItem(const FMassReplicatedAgentHandle Handle)
{
	if (AgentHandleManager.IsValidHandle(Handle) == false)
	{
		return nullptr;
	}

	if (Agents == nullptr)
	{
		return nullptr;
	}

	const FMassAgentLookupData& LookUpData = AgentLookupArray[Handle.GetIndex()];
	return &(*Agents)[LookUpData.AgentsIdx];
}

void FDroneClientBubbleHandler::MarkItemDirty(FDroneFastArrayItem& Item) const
{
	if (Serializer == nullptr)
	{
		return;
	}

	Serializer->MarkItemDirty(Item);
}
#endif

void FDroneClientBubbleHandler::ApplyReplicatedTransform(const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent) const
{
	FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
	FTransform& Transform = TransformFragment.GetMutableTransform();

	Transform.SetLocation(ReplicatedAgent.GetPosition());

	const float YawDeg = FMath::RadiansToDegrees(ReplicatedAgent.GetYawRadians());
	Transform.SetRotation(FQuat(FRotator(0.0f, YawDeg, 0.0f)));
}