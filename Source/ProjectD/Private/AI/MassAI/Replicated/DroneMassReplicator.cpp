#include "AI/MassAI/Replicated/DroneMassReplicator.h"
#include "AI/MassAI/Replicated/DroneClientBubbleInfo.h"
#include "AI/MassAI/Replicated/DroneClientBubbleSerializer.h"
#include "AI/MassAI/Replicated/DroneClientBubbleHandler.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"
#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"
#include "AI/MassAI/Replicated/MassEntityTags.h"
#include "AI/MassAI/Replicated/DeathScheduleFragment.h"
#include "AI/MassAI/MassEntityCueID.h"
#include "ProjectD/ProjectD.h"
#include "MassCommonFragments.h"
#include "MassReplicationFragments.h"
#include "MassReplicationSubsystem.h"
#include "MassLODTypes.h"

#define DLOG(Format, ...) UE_LOG(LogProjectD, Warning, TEXT("[F=%u] " Format), (uint32)GFrameCounter, ##__VA_ARGS__)

void UDroneMassReplicator::AddRequirements(FMassEntityQuery& EntityQuery)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FDeathScheduleFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMassEntityPendingRemovalTag>(EMassFragmentPresence::None);
}

void UDroneMassReplicator::ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext)
{
#if UE_REPLICATION_COMPILE_SERVER_CODE

	FMassReplicationSharedFragment* RepSharedFrag = nullptr;
	TConstArrayView<FTransformFragment> TransformFragments;
	TConstArrayView<FDeathScheduleFragment> DeathSchedules;

	auto CacheViewsCallback = [&]
	(
		FMassExecutionContext& InContext
		)
		{
			TransformFragments = InContext.GetFragmentView<FTransformFragment>();
			DeathSchedules = InContext.GetFragmentView<FDeathScheduleFragment>();
			RepSharedFrag = &InContext.GetMutableSharedFragment<FMassReplicationSharedFragment>();
		};

	auto AddEntityCallback = [&]
	(
		FMassExecutionContext& InContext,
		const int32 EntityIdx,
		FDroneReplicatedAgent& InReplicatedAgent,
		const FMassClientHandle ClientHandle
		) -> FMassReplicatedAgentHandle
		{
			ADroneClientBubbleInfo& BubbleInfo =
				RepSharedFrag->GetTypedClientBubbleInfoChecked<ADroneClientBubbleInfo>(ClientHandle);

			FDroneClientBubbleHandler& Bubble =
				BubbleInfo.GetSerializerMutable().GetBubbleHandlerMutable();

			const FTransform& Xf = TransformFragments[EntityIdx].GetTransform();
			const FVector Pos = Xf.GetLocation();
			const float YawRad = FMath::DegreesToRadians(Xf.Rotator().Yaw);

			InReplicatedAgent.GetPositionYawMutable().SetPosition(Pos);
			InReplicatedAgent.GetPositionYawMutable().SetYaw(YawRad);

			const FDeathScheduleFragment& Schedule = DeathSchedules[EntityIdx];
			if (Schedule.RemoveAtFrame != 0)
			{
				InReplicatedAgent.SetDead(Schedule.DeathLoc, EMassEntityCueId::Drone_Death);
			}

			const FMassEntityHandle EntityHandle = InContext.GetEntity(EntityIdx);
			const FMassReplicatedAgentHandle Handle = Bubble.AddAgent(EntityHandle, InReplicatedAgent);

			FDroneFastArrayItem* Item = Bubble.GetMutableItem(Handle);
			if (Item != nullptr)
			{
				const FMassNetworkID NetID = Item->Agent.GetNetID();
				if (NetID.IsValid() == true)
				{
					Bubble.RegisterNetIdHandle(NetID, Handle);
				}
			}

			return Handle;
		};

	auto ModifyEntityCallback = [&]
	(
		FMassExecutionContext& InContext,
		const int32 EntityIdx,
		const EMassLOD::Type LOD,
		const double Time,
		const FMassReplicatedAgentHandle Handle,
		const FMassClientHandle ClientHandle
		)
		{
			ADroneClientBubbleInfo& BubbleInfo =
				RepSharedFrag->GetTypedClientBubbleInfoChecked<ADroneClientBubbleInfo>(ClientHandle);

			FDroneClientBubbleHandler& Bubble =
				BubbleInfo.GetSerializerMutable().GetBubbleHandlerMutable();

			FDroneFastArrayItem* Item = Bubble.GetMutableItem(Handle);
			if (Item == nullptr)
			{
				return;
			}

			{
				const FMassNetworkID NetID = Item->Agent.GetNetID();
				if (NetID.IsValid() == true)
				{
					Bubble.RegisterNetIdHandle(NetID, Handle);
				}
			}

			bool bMarkItemDirty = false;

			const FTransform& Xf = TransformFragments[EntityIdx].GetTransform();
			const FVector Pos = Xf.GetLocation();
			const float YawRad = FMath::DegreesToRadians(Xf.Rotator().Yaw);

			constexpr float PositionTolerance = 10.0f;
			if (FVector::PointsAreNear(Pos, Item->Agent.GetPosition(), PositionTolerance) == false)
			{
				Item->Agent.GetPositionYawMutable().SetPosition(Pos);
				bMarkItemDirty = true;
			}

			constexpr float YawTolerance = 0.01f;
			if (FMath::IsNearlyEqual(YawRad, Item->Agent.GetYawRadians(), YawTolerance) == false)
			{
				Item->Agent.GetPositionYawMutable().SetYaw(YawRad);
				bMarkItemDirty = true;
			}

			const FDeathScheduleFragment& Schedule = DeathSchedules[EntityIdx];
			const bool bShouldBeDead = (Schedule.RemoveAtFrame != 0);

			if ((bShouldBeDead == true) && (Item->Agent.GetIsDead() == false))
			{
				Item->Agent.SetDead(Schedule.DeathLoc, EMassEntityCueId::Drone_Death);
				bMarkItemDirty = true;

				DLOG("[RepModify] Client=%d NetID=%u => SetDead Loc=%s",
					ClientHandle.GetIndex(),
					(uint32)Item->Agent.GetNetID().GetValue(),
					*FVector(Schedule.DeathLoc).ToString());
			}

			if (bMarkItemDirty == true)
			{
				Bubble.MarkItemDirty(*Item);
			}
		};

	auto RemoveEntityCallback = [&]
	(
		FMassExecutionContext& InContext,
		const FMassReplicatedAgentHandle Handle,
		const FMassClientHandle ClientHandle
		)
		{
			ADroneClientBubbleInfo& BubbleInfo =
				RepSharedFrag->GetTypedClientBubbleInfoChecked<ADroneClientBubbleInfo>(ClientHandle);

			FDroneClientBubbleHandler& Bubble =
				BubbleInfo.GetSerializerMutable().GetBubbleHandlerMutable();

			FDroneFastArrayItem* Item = Bubble.GetMutableItem(Handle);
			if (Item == nullptr)
			{
				DLOG("[ServerRemoveCB] Client=%d HandleIdx=%d Item=NULL", ClientHandle.GetIndex(), Handle.GetIndex());
				return;
			}

			const uint32 NetID = (uint32)Item->Agent.GetNetID().GetValue();
			const bool bRemoved = Bubble.CleanAgent(Handle);

			DLOG("[ServerRemoveCB] Client=%d NetID=%u => CleanAgent=%d",
				ClientHandle.GetIndex(),
				NetID,
				(bRemoved == true) ? 1 : 0);
		};

	CalculateClientReplication<FDroneFastArrayItem>(
		Context,
		ReplicationContext,
		CacheViewsCallback,
		AddEntityCallback,
		ModifyEntityCallback,
		RemoveEntityCallback
	);

	if (RepSharedFrag == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[DroneRep] RepSharedFrag NULL - skip event flush"));
		return;
	}

#endif
}
