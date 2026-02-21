#include "AI/MassAI/Replicated/DroneClientBubbleHandler.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"
#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"
#include "AI/MassAI/MassEntityEffectSubsystem.h"
#include "MassClientBubbleHandler.h"
#include "ProjectD/ProjectD.h"

#define DLOG(Format, ...) UE_LOG(LogProjectD, Warning, TEXT("[F=%u] " Format), (uint32)GFrameCounter, ##__VA_ARGS__)

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

void FDroneClientBubbleHandler::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	using Super = TClientBubbleHandlerBase<FDroneFastArrayItem>;

	UWorld* World = (EffectSubsystem != nullptr) ? EffectSubsystem->GetWorld() : nullptr;
	const ENetMode NetMode = IsValid(World) ? World->GetNetMode() : NM_MAX;

	if ((NetMode == NM_Client) &&
		(Agents != nullptr))
	{
		for (const int32 Idx : RemovedIndices)
		{
			if (Agents->IsValidIndex(Idx) == false)
			{
				DLOG("[ClientPreRemove] InvalidIndex idx=%d", Idx);
				continue;
			}

			const FDroneFastArrayItem& Item = (*Agents)[Idx];
			const FDroneReplicatedAgent& Agent = Item.Agent;

			const uint32 NetID = (uint32)Agent.GetNetID().GetValue();
			const bool bDead = (Agent.GetIsDead() == true);
			const EMassEntityCueId Cue = Agent.GetCueId();

			const FVector Pos = Agent.GetPosition();
			const FVector DeathLoc = Agent.GetDeathLocation();
			const FVector UseLoc = (DeathLoc.IsNearlyZero() == false) ? DeathLoc : Pos;

			DLOG("[ClientPreRemove] idx=%d NetID=%u Dead=%d Cue=%d Pos=%s DeathLoc=%s UseLoc=%s",
				Idx,
				NetID,
				bDead ? 1 : 0,
				Cue,
				*Pos.ToString(),
				*DeathLoc.ToString(),
				*UseLoc.ToString());

			if ((bDead == true) && 
				(EffectSubsystem != nullptr) 
				&& (Cue != EMassEntityCueId::None))
			{
				EffectSubsystem->PlayCueAtLocation(Cue, UseLoc);
				DLOG("[ClientPreRemove] FX Played NetID=%u Cue=%d", NetID, Cue);
			}
		}
	}

	Super::PreReplicatedRemove(RemovedIndices, FinalSize);
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

bool FDroneClientBubbleHandler::CleanAgent(const FMassReplicatedAgentHandle Handle)
{
	using Super = TClientBubbleHandlerBase<FDroneFastArrayItem>;

	const bool bRemoved = Super::RemoveAgent(Handle);

	DLOG("[DroneBubble][Server] CleanAgent handleIdx=%d removed=%d",
		Handle.GetIndex(),
		(bRemoved == true) ? 1 : 0);

	if ((bRemoved == true) && (Serializer != nullptr))
	{
		Serializer->MarkArrayDirty();
	}

	return bRemoved;
}
#endif // UE_REPLICATION_COMPILE_SERVER_CODE