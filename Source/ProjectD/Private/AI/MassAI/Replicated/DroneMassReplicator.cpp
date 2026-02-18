#include "AI/MassAI/Replicated/DroneMassReplicator.h"
#include "AI/MassAI/Replicated/DroneClientBubbleInfo.h"
#include "AI/MassAI/Replicated/DroneClientBubbleSerializer.h"
#include "AI/MassAI/Replicated/DroneClientBubbleHandler.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"
#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"
#include "AI/MassAI/Replicated/DroneEventQueueSubsystem.h"
#include "AI/MassAI/Replicated/MassEntityTags.h"
#include "ProjectD/ProjectD.h"
#include "MassCommonFragments.h"
#include "MassReplicationFragments.h"
#include "MassReplicationSubsystem.h"
#include "MassLODTypes.h"

void UDroneMassReplicator::AddRequirements(FMassEntityQuery& EntityQuery)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMassEntityDyingTag>(EMassFragmentPresence::None);
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

			const FMassEntityHandle EntityHandle = InContext.GetEntity(EntityIdx);
			const FMassReplicatedAgentHandle Handle = Bubble.AddAgent(EntityHandle, InReplicatedAgent);

			// NetID -> Handle 등록 (Add 직후)
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
			if (Item != nullptr)
			{
				const uint32 NetIDValue = (uint32)Item->Agent.GetNetID().GetValue();
				UE_LOG(LogProjectD, Warning, TEXT("[ServerRemoveCB] net=%u rep=%d handleIdx=%d"),
					NetIDValue,
					Item->ReplicationID,
					Handle.GetIndex());
			}
			else
			{
				UE_LOG(LogProjectD, Warning, TEXT("[ServerRemoveCB] Item NULL handleIdx=%d"), Handle.GetIndex());
			}

			// LOD/버블 변경으로 빠질 때 정상 Remove 경로
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

	UWorld* World = Context.GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	UDroneEventQueueSubsystem* EventQ = World->GetSubsystem<UDroneEventQueueSubsystem>();
	if (IsValid(EventQ) == false)
	{
		return;
	}

	UMassReplicationSubsystem* RepSub = World->GetSubsystem<UMassReplicationSubsystem>();
	if (IsValid(RepSub) == false)
	{
		return;
	}

	const TArray<FMassClientHandle>& Clients = RepSub->GetClientReplicationHandles();

	{
		TArray<FDroneDeathEvent> Deaths;
		EventQ->MoveDeathArray(Deaths);

		if (Deaths.Num() > 0)
		{
			for (const FMassClientHandle ClientHandle : Clients)
			{
				ADroneClientBubbleInfo& BubbleInfo =
					RepSharedFrag->GetTypedClientBubbleInfoChecked<ADroneClientBubbleInfo>(ClientHandle);

				FDroneClientBubbleHandler& Bubble =
					BubbleInfo.GetSerializerMutable().GetBubbleHandlerMutable();

				bool bAnyDirty = false;

				for (const FDroneDeathEvent& E : Deaths)
				{
					if (E.NetID.IsValid() == false)
					{
						continue;
					}

					const bool bDirty = Bubble.MarkDeadByNetId(E.NetID, E.DeathLocation, E.CueId);
					if (bDirty == true)
					{
						bAnyDirty = true;
					}
				}

				if (bAnyDirty == true)
				{
					BubbleInfo.ForceNetUpdate();
				}
			}
		}
	}

	{
		TArray<FMassNetworkID> DueRemovals;
		EventQ->MoveDueRemovals(DueRemovals);

		if (DueRemovals.Num() > 0)
		{
			for (const FMassClientHandle ClientHandle : Clients)
			{
				ADroneClientBubbleInfo& BubbleInfo =
					RepSharedFrag->GetTypedClientBubbleInfoChecked<ADroneClientBubbleInfo>(ClientHandle);

				FDroneClientBubbleHandler& Bubble =
					BubbleInfo.GetSerializerMutable().GetBubbleHandlerMutable();

				bool bAnyRemoved = false;

				for (const FMassNetworkID NetID : DueRemovals)
				{
					if (NetID.IsValid() == false)
					{
						continue;
					}

					const bool bRemoved = Bubble.RemoveByNetId(NetID);
					if (bRemoved == true)
					{
						bAnyRemoved = true;
					}
				}

				if (bAnyRemoved == true)
				{
					BubbleInfo.ForceNetUpdate();
				}
			}
		}
	}

#endif
}
