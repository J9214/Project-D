#include "AI/MassAI/MassBoidsDestructionProcessor.h"
#include "AI/MassAI/MassBoidsHealthFragment.h"
#include "AI/MassAI/MassDamageBridgeSubsystem.h"
#include "AI/MassAI/MassProxyPoolSubsystem.h"
#include "AI/MassAI/Replicated/MassEntityTags.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassEntityView.h"
#include "MassReplicationFragments.h"
#include "AI/MassAI/Replicated/DeathScheduleFragment.h"
#include "ProjectD/ProjectD.h"

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
	EntityQuery.AddRequirement<FMassBoidsHealthFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FDeathScheduleFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassEntityDyingTag>(EMassFragmentPresence::None);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsDestructionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
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

	UMassProxyPoolSubsystem* Pool = World->GetSubsystem<UMassProxyPoolSubsystem>();

	EntityQuery.ForEachEntityChunk(Context, [this, Pool](FMassExecutionContext& ExecContext)
		{
			const TArrayView<FMassBoidsHealthFragment> Healths = ExecContext.GetMutableFragmentView<FMassBoidsHealthFragment>();
			const TConstArrayView<FTransformFragment> Transforms = ExecContext.GetFragmentView<FTransformFragment>();
			TArrayView<FDeathScheduleFragment> Schedules = ExecContext.GetMutableFragmentView<FDeathScheduleFragment>();

			const uint32 NowFrame = (uint32)GFrameCounter;

			constexpr uint32 RemoveDelayFrames = 2;
			constexpr uint32 DestroyDelayFrames = 4;

			const int32 NumEntities = ExecContext.GetNumEntities();
			for (int32 i = 0; i < NumEntities; ++i)
			{
				if (Healths[i].Health > 0.0f)
				{
					continue;
				}

				const FMassEntityHandle Entity = ExecContext.GetEntity(i);
				FDeathScheduleFragment& Schedule = Schedules[i];

				if (Schedule.RemoveAtFrame != 0)
				{
					continue;
				}

				Schedule.RemoveAtFrame = NowFrame + RemoveDelayFrames;
				Schedule.DestroyAtFrame = NowFrame + DestroyDelayFrames;
				Schedule.DeathLoc = Transforms[i].GetTransform().GetLocation();
				Schedule.bMarkedForRemoval = false;

				ExecContext.Defer().AddTag<FMassEntityDyingTag>(Entity);

				if (IsValid(Pool) == true)
				{
					Pool->Release(Entity);
				}

				UE_LOG(LogProjectD, Warning, TEXT("[F=%u][DeathSchedule] Entity=%d RemoveAt=%u DestroyAt=%u Loc=%s"),
					(uint32)GFrameCounter,
					Entity.Index,
					Schedule.RemoveAtFrame,
					Schedule.DestroyAtFrame,
					*FVector(Schedule.DeathLoc).ToString());
			}
		});
}