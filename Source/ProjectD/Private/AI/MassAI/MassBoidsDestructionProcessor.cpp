#include "AI/MassAI/MassBoidsDestructionProcessor.h"
#include "AI/MassAI/MassBoidsHealthFragment.h"
#include "AI/MassAI/MassDamageBridgeSubsystem.h"
#include "AI/MassAI/MassProxyPoolSubsystem.h"
#include "AI/MassAI/Replicated/MassEntityTags.h"
#include "AI/MassAI/Replicated/DroneEventQueueSubsystem.h"
#include "AI/MassAI/MassEntityCueId.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassEntityView.h"
#include "MassReplicationFragments.h"

UMassBoidsDestructionProcessor::UMassBoidsDestructionProcessor()
	:EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UMassBoidsDestructionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassBoidsHealthFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMassEntityDyingTag>(EMassFragmentPresence::None);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsDestructionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	for (const FMassEntityHandle& E : PendingDestroyEntities)
	{
		if (EntityManager.IsEntityValid(E) == true)
		{
			Context.Defer().DestroyEntity(E);
		}
	}
	PendingDestroyEntities.Reset();

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	UMassDamageBridgeSubsystem* Bridge = World->GetSubsystem<UMassDamageBridgeSubsystem>();

	if (IsValid(Bridge) == true)
	{
		TArray<FPendingMassDamage> Requests;
		Bridge->MovePendingDamages(Requests);

		for (const FPendingMassDamage& Req : Requests)
		{
			if (Req.Entity.IsValid() == false ||
				Req.Damage <= 0.0f)
			{
				continue;
			}

			if (EntityManager.IsEntityValid(Req.Entity) == false)
			{
				continue;
			}

			FMassEntityView View = FMassEntityView::TryMakeView(EntityManager, Req.Entity);
			if (View.IsValid() == false)
			{
				continue;
			}

			FMassBoidsHealthFragment* HealthFrag = View.GetFragmentDataPtr<FMassBoidsHealthFragment>();
			if (HealthFrag == nullptr)
			{
				continue;
			}

			HealthFrag->Health = FMath::Clamp(HealthFrag->Health - Req.Damage, 0.0f, HealthFrag->MaxHealth);
		}
	}

	UDroneEventQueueSubsystem* EventSub = World->GetSubsystem<UDroneEventQueueSubsystem>();
	UMassProxyPoolSubsystem* Pool = World->GetSubsystem<UMassProxyPoolSubsystem>();

	if ((IsValid(EventSub) == true) &&
		(IsValid(Pool) == true))
	{
		EntityQuery.ForEachEntityChunk(Context, [this, EventSub, Pool](FMassExecutionContext& ExecContext)
			{
				const TConstArrayView<FMassBoidsHealthFragment> Healths = ExecContext.GetFragmentView<FMassBoidsHealthFragment>();
				const TConstArrayView<FTransformFragment> Transforms = ExecContext.GetFragmentView<FTransformFragment>();

				const int32 NumEntities = ExecContext.GetNumEntities();
				for (int32 i = 0; i < NumEntities; ++i)
				{
					if (Healths[i].Health > 0.0f)
					{
						continue;
					}

					const FMassEntityHandle Entity = ExecContext.GetEntity(i);

					FMassNetworkID NetID;
					{
						const FMassEntityView View = FMassEntityView::TryMakeView(ExecContext.GetEntityManagerChecked(), Entity);
						if (View.IsValid() == true)
						{
							const FMassNetworkIDFragment* NetFrag = View.GetFragmentDataPtr<FMassNetworkIDFragment>();
							if (NetFrag != nullptr)
							{
								NetID = NetFrag->NetID;
							}
						}
					}

					const FVector DeathLocation = Transforms[i].GetTransform().GetLocation();

					if (IsValid(EventSub) == true &&
						NetID.IsValid() == true)
					{
						// if need CudId Define Enum
						EventSub->EnqueueDeathEvent(NetID, DeathLocation, EMassEntityCueId::Drone_Death);
					}

					ExecContext.Defer().AddTag<FMassEntityDyingTag>(Entity);

					if (IsValid(Pool) == true)
					{
						Pool->Release(Entity);
					}

					PendingDestroyEntities.Add(Entity);
				}
			});
	}
}