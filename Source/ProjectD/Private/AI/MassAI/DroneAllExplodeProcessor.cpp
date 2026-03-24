#include "AI/MassAI/DroneAllExplodeProcessor.h"
#include "AI/MassAI/CheckDroneAllExplodeSubsystem.h"
#include "AI/MassAI/Replicated/MassEntityTags.h"
#include "AI/MassAI/MassBoidsHealthFragment.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "ProjectD/ProjectD.h"

UDroneAllExplodeProcessor::UDroneAllExplodeProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UDroneAllExplodeProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassBoidsHealthFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddTagRequirement<FMassEntitySuicideTag>(EMassFragmentPresence::None);
	EntityQuery.AddTagRequirement<FMassEntityDyingTag>(EMassFragmentPresence::None);
	EntityQuery.AddTagRequirement<FMassEntityPendingRemovalTag>(EMassFragmentPresence::None);

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

	UE_LOG(LogProjectD, Warning, TEXT("[UDroneAllExplodeProcessor] Drone Explose!"));

	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& ExecContext)
		{
			TArrayView<FMassVelocityFragment> Velocities = ExecContext.GetMutableFragmentView<FMassVelocityFragment>();
			TArrayView<FMassBoidsHealthFragment> HealthInfos = ExecContext.GetMutableFragmentView<FMassBoidsHealthFragment>();

			const int32 NumEntities = ExecContext.GetNumEntities();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FVector& Velocity = Velocities[i].Value;
				FMassBoidsHealthFragment& HealthInfo = HealthInfos[i];

				if (HealthInfo.Health <= 0)
				{
					continue;
				}

				HealthInfo.Health = 0.0f;
				Velocity = FVector::ZeroVector;

				const FMassEntityHandle Entity = ExecContext.GetEntity(i);
				ExecContext.Defer().AddTag<FMassEntitySuicideTag>(Entity);
			}
		});
}