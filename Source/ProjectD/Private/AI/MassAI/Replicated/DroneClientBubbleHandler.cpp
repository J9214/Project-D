#include "AI/MassAI/Replicated/DroneClientBubbleHandler.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"
#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"
#include "AI/MassAI/MassEntityEffectSubsystem.h"
#include "MassClientBubbleHandler.h"

FDroneClientBubbleHandler::FDroneClientBubbleHandler()
	: TClientBubbleHandlerBase<FDroneFastArrayItem>()
{
}

void FDroneClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	auto AddRequirementsForSpawnQuery = [](FMassEntityQuery& Query)
		{
			Query.AddRequirement<FClientVisualFragment>(EMassFragmentAccess::ReadWrite);
		};

	auto CacheFragmentViewsForSpawnQuery = [](FMassExecutionContext& Context)
		{
		};

	auto SetSpawnedEntityData = [this](const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent, const int32 EntityIdx)
		{
			ApplyReplicatedVisualState(EntityView, ReplicatedAgent, true);
		};

	auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent)
		{
			ApplyReplicatedVisualState(EntityView, ReplicatedAgent, true);
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
			TryPlayDeathCueOnce(ReplicatedAgent);
			ApplyReplicatedVisualState(EntityView, ReplicatedAgent, false);
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
				continue;
			}

			const FDroneFastArrayItem& Item = (*Agents)[Idx];
			const FDroneReplicatedAgent& Agent = Item.Agent;
			const uint32 NetID = (uint32)Agent.GetNetID().GetValue();

			PlayedDeathFxNetIDs.Remove(NetID);
		}
	}

	Super::PreReplicatedRemove(RemovedIndices, FinalSize);
}

void FDroneClientBubbleHandler::ApplyReplicatedVisualState(const FMassEntityView& EntityView, const FDroneReplicatedAgent& ReplicatedAgent, const bool bForceSnap) const
{
	FClientVisualFragment& VisualTarget = EntityView.GetFragmentData<FClientVisualFragment>();

	const FVector Pos = ReplicatedAgent.GetPosition();
	const float YawDeg = FMath::RadiansToDegrees(ReplicatedAgent.GetYawRadians());

	VisualTarget.TargetLocation = Pos;
	VisualTarget.TargetRotation = FQuat(FRotator(0.0f, YawDeg, 0.0f));

	const bool bDead = (ReplicatedAgent.GetIsDead() == true);
	VisualTarget.bDead = bDead;

	if (bDead == true)
	{
		const FVector DeathLoc = ReplicatedAgent.GetDeathLocation();
		VisualTarget.DeathLocation = (DeathLoc.IsNearlyZero() == false) ? DeathLoc : Pos;

		VisualTarget.bSnapThisFrame = true;
	}
	else
	{
		VisualTarget.DeathLocation = FVector::ZeroVector;

		if ((bForceSnap == true) || (VisualTarget.bInitialized == false))
		{
			VisualTarget.bSnapThisFrame = true;
		}
	}

	if (VisualTarget.bInitialized == false)
	{
		VisualTarget.bInitialized = true;
	}
}

void FDroneClientBubbleHandler::TryPlayDeathCueOnce(const FDroneReplicatedAgent& ReplicatedAgent)
{
	UWorld* World = (EffectSubsystem != nullptr) ? EffectSubsystem->GetWorld() : nullptr;
	const ENetMode NetMode = IsValid(World) ? World->GetNetMode() : NM_MAX;

	if (NetMode != NM_Client)
	{
		return;
	}

	const uint32 NetID = (uint32)ReplicatedAgent.GetNetID().GetValue();
	const bool bDead = (ReplicatedAgent.GetIsDead() == true);
	const EMassEntityCueId Cue = ReplicatedAgent.GetCueId();

	if (bDead == false)
	{
		PlayedDeathFxNetIDs.Remove(NetID);
		return;
	}

	if (Cue == EMassEntityCueId::None)
	{
		return;
	}

	if (IsValid(EffectSubsystem) == false)
	{
		return;
	}

	if (PlayedDeathFxNetIDs.Contains(NetID) == true)
	{
		return;
	}

	const FVector Pos = ReplicatedAgent.GetPosition();
	const FVector DeathLoc = ReplicatedAgent.GetDeathLocation();
	const FVector UseLoc = (DeathLoc.IsNearlyZero() == false) ? DeathLoc : Pos;

	EffectSubsystem->PlayCueAtLocation(Cue, UseLoc);
	PlayedDeathFxNetIDs.Add(NetID);
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

	if ((bRemoved == true) &&
		(Serializer != nullptr))
	{
		Serializer->MarkArrayDirty();
	}

	return bRemoved;
}
#endif // UE_REPLICATION_COMPILE_SERVER_CODE