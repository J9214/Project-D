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
#include "MassLODTypes.h"

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
				const EMassEntityCueId CueId =
					(Schedule.DeathCueId != EMassEntityCueId::None) ? Schedule.DeathCueId : EMassEntityCueId::Drone_Death;

				InReplicatedAgent.SetDead(Schedule.DeathLoc, CueId);
			}

			const FMassEntityHandle EntityHandle = InContext.GetEntity(EntityIdx);
			return Bubble.AddAgent(EntityHandle, InReplicatedAgent);
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
				Item->Agent.SetDead(Schedule.DeathLoc, Schedule.DeathCueId);
				bMarkItemDirty = true;
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

			Bubble.CleanAgent(Handle);
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
