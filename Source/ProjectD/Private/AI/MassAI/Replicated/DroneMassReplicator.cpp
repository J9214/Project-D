#include "AI/MassAI/Replicated/DroneMassReplicator.h"
#include "AI/MassAI/Replicated/DroneClientBubbleInfo.h"
#include "AI/MassAI/Replicated/DroneClientBubbleSerializer.h"
#include "AI/MassAI/Replicated/DroneClientBubbleHandler.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"
#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"

#include "MassCommonFragments.h"
#include "MassReplicationFragments.h"
#include "MassLODTypes.h"

void UDroneMassReplicator::AddRequirements(FMassEntityQuery& EntityQuery)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UDroneMassReplicator::ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext)
{
#if UE_REPLICATION_COMPILE_SERVER_CODE

	FMassReplicationSharedFragment* RepSharedFrag = nullptr;
	TConstArrayView<FTransformFragment> TransformFragments;

	auto CacheViewsCallback = [&]
	(
		FMassExecutionContext& InContext
		)
		{
			TransformFragments = InContext.GetFragmentView<FTransformFragment>();
			RepSharedFrag = &InContext.GetMutableSharedFragment<FMassReplicationSharedFragment>();
		};

	auto AddEntityCallback = [&]
	(
		FMassExecutionContext& InContext,
		const int32 EntityIdx,
		FDroneReplicatedAgent& InReplicatedAgent,
		const FMassClientHandle ClientHandle
		)
		{
			ADroneClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<ADroneClientBubbleInfo>(ClientHandle);

			const FTransform& Xf = TransformFragments[EntityIdx].GetTransform();
			const FVector Pos = Xf.GetLocation();
			const float YawRad = FMath::DegreesToRadians(Xf.Rotator().Yaw);

			InReplicatedAgent.GetPositionYawMutable().SetPosition(Pos);
			InReplicatedAgent.GetPositionYawMutable().SetYaw(YawRad);

			return BubbleInfo.GetSerializerMutable().GetBubbleHandlerMutable().AddAgent(InContext.GetEntity(EntityIdx), InReplicatedAgent);
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
			ADroneClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<ADroneClientBubbleInfo>(ClientHandle);
			FDroneClientBubbleHandler& Bubble = BubbleInfo.GetSerializerMutable().GetBubbleHandlerMutable();

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
			ADroneClientBubbleInfo& BubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<ADroneClientBubbleInfo>(ClientHandle);
			BubbleInfo.GetSerializerMutable().GetBubbleHandlerMutable().RemoveAgent(Handle);
		};

	CalculateClientReplication<FDroneFastArrayItem>(Context, ReplicationContext, CacheViewsCallback, AddEntityCallback, ModifyEntityCallback, RemoveEntityCallback);

#endif
}
