#include "AI/MassAI/DroneAllExplodeProcessor.h"
#include "AI/MassAI/CheckDroneAllExploseSubsystem.h"
#include "AI/MassAI/Replicated/MassEntityTags.h"
#include "MassExecutionContext.h"

UDroneAllExplodeProcessor::UDroneAllExplodeProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UDroneAllExplodeProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FMassEntitySuicideTag>(EMassFragmentPresence::None);
	EntityQuery.AddTagRequirement<FMassEntityDyingTag>(EMassFragmentPresence::None);

	EntityQuery.RegisterWithProcessor(*this);
}

void UDroneAllExplodeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	UCheckDroneAllExplodeSubsystem* AllExplodeSubsystem = World->GetSubsystem<UCheckDroneAllExplodeSubsystem>();
	if (IsValid(AllExplodeSubsystem) == false)
	{
		return;
	}

	if (AllExplodeSubsystem->ConsumeExplodeAllRequest() == false)
	{
		return;
	}

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& ExecContext)
		{
			const int32 NumEntities = ExecContext.GetNumEntities();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FMassEntityHandle Entity = ExecContext.GetEntity(i);
				ExecContext.Defer().AddTag<FMassEntitySuicideTag>(Entity);
			}
		});
}