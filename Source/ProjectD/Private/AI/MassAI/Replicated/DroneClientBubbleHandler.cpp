#include "AI/MassAI/Replicated/DroneClientBubbleHandler.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"
#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"
#include "AI/MassAI/MassEntityEffectSubsystem.h"
#include "MassClientBubbleHandler.h"
#include "ProjectD/ProjectD.h"

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

void FDroneClientBubbleHandler::InitializeForWorld(UWorld& World)
{
	using Super = TClientBubbleHandlerBase<FDroneFastArrayItem>;

	Super::InitializeForWorld(World);

	EffectSubsystem = World.GetSubsystem<UMassEntityEffectSubsystem>();
}

void FDroneClientBubbleHandler::ApplyReplicatedTransform(const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent) const
{
	FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();
	FTransform& Transform = TransformFragment.GetMutableTransform();

	Transform.SetLocation(ReplicatedAgent.GetPosition());

	const float YawDeg = FMath::RadiansToDegrees(ReplicatedAgent.GetYawRadians());
	Transform.SetRotation(FQuat(FRotator(0.0f, YawDeg, 0.0f)));
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

void FDroneClientBubbleHandler::RegisterNetIdHandle(const FMassNetworkID NetID, const FMassReplicatedAgentHandle Handle)
{
	const uint32 Key = (uint32)NetID.GetValue();
	if (Key == 0)
	{
		return;
	}

	NetIdToHandle.Add(Key, Handle);
}

bool FDroneClientBubbleHandler::MarkDeadByNetId(const FMassNetworkID NetID, const FVector_NetQuantize& DeathLoc, const EMassEntityCueId CueId)
{
	if ((Agents == nullptr) ||
		(Serializer == nullptr))
	{
		return false;
	}

	const uint32 Key = (uint32)NetID.GetValue();
	if (Key == 0)
	{
		return false;
	}

	const FMassReplicatedAgentHandle* HandlePtr = NetIdToHandle.Find(Key);
	if (HandlePtr == nullptr)
	{
		return false;
	}

	FDroneFastArrayItem* Item = GetMutableItem(*HandlePtr);
	if (Item == nullptr)
	{
		return false;
	}

	Item->Agent.SetDead(DeathLoc, CueId);

	Serializer->MarkItemDirty(*Item);
	return true;
}

bool FDroneClientBubbleHandler::RemoveByNetId(const FMassNetworkID NetID)
{
	const uint32 Key = (uint32)NetID.GetValue();
	if (Key == 0)
	{
		return false;
	}

	const FMassReplicatedAgentHandle* HandlePtr = NetIdToHandle.Find(Key);
	if (HandlePtr == nullptr)
	{
		return false;
	}

	return CleanAgent(*HandlePtr);
}

bool FDroneClientBubbleHandler::CleanAgent(const FMassReplicatedAgentHandle Handle)
{
	using Super = TClientBubbleHandlerBase<FDroneFastArrayItem>;

	uint32 NetKey = 0;
	{
		FDroneFastArrayItem* Item = GetMutableItem(Handle);
		if (Item != nullptr)
		{
			NetKey = (uint32)Item->Agent.GetNetID().GetValue();
		}
	}

	const bool bRemoved = Super::RemoveAgent(Handle);

	UE_LOG(LogProjectD, Warning, TEXT("[DroneBubble] CleanAgent handleIdx=%d removed=%d"),
		Handle.GetIndex(),
		(bRemoved == true) ? 1 : 0);

	if (bRemoved == true)
	{
		if (NetKey != 0)
		{
			NetIdToHandle.Remove(NetKey);
		}

		if (Serializer != nullptr)
		{
			Serializer->MarkArrayDirty();
		}
	}

	return bRemoved;
}

#endif // UE_REPLICATION_COMPILE_SERVER_CODE