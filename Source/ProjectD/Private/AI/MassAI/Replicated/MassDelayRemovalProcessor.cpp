#include "AI/MassAI/Replicated/MassDelayRemovalProcessor.h"
#include "AI/MassAI/Replicated/DeathScheduleFragment.h"
#include "AI/MassAI/Replicated/MassEntityTags.h"
#include "MassExecutionContext.h"
#include "ProjectD/ProjectD.h"
#include "Engine/World.h"

UMassDelayRemovalProcessor::UMassDelayRemovalProcessor()
	:EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UMassDelayRemovalProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FDeathScheduleFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassEntityDyingTag>(EMassFragmentPresence::All);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassDelayRemovalProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const uint32 NowFrame = (uint32)GFrameCounter;

	EntityQuery.ForEachEntityChunk(Context,
		[this, NowFrame](FMassExecutionContext& ExecContext)
		{
			TArrayView<FDeathScheduleFragment> Schedules = ExecContext.GetMutableFragmentView<FDeathScheduleFragment>();

			const int32 NumEntities = ExecContext.GetNumEntities();
			for (int32 i = 0; i < NumEntities; ++i)
			{
				FDeathScheduleFragment& S = Schedules[i];
				const FMassEntityHandle Entity = ExecContext.GetEntity(i);

				if ((S.bMarkedForRemoval == false) &&
					(S.RemoveAtFrame != 0) &&
					(NowFrame >= S.RemoveAtFrame))
				{
					S.bMarkedForRemoval = true;
					ExecContext.Defer().AddTag<FMassEntityPendingRemovalTag>(Entity);
				}

				if ((S.DestroyAtFrame != 0) &&
					(NowFrame >= S.DestroyAtFrame))
				{
					ExecContext.Defer().DestroyEntity(Entity);
				}
			}
		});
}
