#include "AI/MassAI/DroneAllExplodeProcessor.h"
#include "AI/MassAI/CheckDroneAllExplodeSubsystem.h"
#include "AI/MassAI/DronePendingExplosionFragment.h"
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
	EntityQuery.AddRequirement<FDronePendingExplosionFragment>(EMassFragmentAccess::ReadWrite);

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

	const uint64 CurrentFrame = GFrameCounter;
	const bool bShouldScheduleExplode = AllExplodeSubsystem->ConsumeExplodeAllRequest();
	const int32 SafeExplodeOverFrames = FMath::Max(1, ExplodeOverFrames);

	if (bShouldScheduleExplode == true)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[UDroneAllExplodeProcessor] Schedule Drone Explode. OverFrames=%d"), SafeExplodeOverFrames);

		int32 GlobalDroneIndex = 0;

		EntityQuery.ForEachEntityChunk(Context, [CurrentFrame, SafeExplodeOverFrames, &GlobalDroneIndex](FMassExecutionContext& ExecContext)
			{
				TArrayView<FMassVelocityFragment> Velocities = ExecContext.GetMutableFragmentView<FMassVelocityFragment>();
				TArrayView<FMassBoidsHealthFragment> HealthInfos = ExecContext.GetMutableFragmentView<FMassBoidsHealthFragment>();
				TArrayView<FDronePendingExplosionFragment> PendingInfos = ExecContext.GetMutableFragmentView<FDronePendingExplosionFragment>();

				const int32 NumEntities = ExecContext.GetNumEntities();

				for (int32 i = 0; i < NumEntities; ++i)
				{
					FVector& Velocity = Velocities[i].Value;
					FMassBoidsHealthFragment& HealthInfo = HealthInfos[i];
					FDronePendingExplosionFragment& PendingInfo = PendingInfos[i];

					if (HealthInfo.Health <= 0.0f)
					{
						continue;
					}

					if (PendingInfo.bPendingExplode == true)
					{
						continue;
					}

					const int32 FrameOffset = GlobalDroneIndex % SafeExplodeOverFrames;

					PendingInfo.bPendingExplode = true;
					PendingInfo.ExplodeAtFrame = CurrentFrame + (uint64)FrameOffset;

					Velocity = FVector::ZeroVector;

					++GlobalDroneIndex;
				}
			});
	}

	EntityQuery.ForEachEntityChunk(Context, [CurrentFrame](FMassExecutionContext& ExecContext)
		{
			TArrayView<FMassVelocityFragment> Velocities = ExecContext.GetMutableFragmentView<FMassVelocityFragment>();
			TArrayView<FMassBoidsHealthFragment> HealthInfos = ExecContext.GetMutableFragmentView<FMassBoidsHealthFragment>();
			TArrayView<FDronePendingExplosionFragment> PendingInfos = ExecContext.GetMutableFragmentView<FDronePendingExplosionFragment>();

			const int32 NumEntities = ExecContext.GetNumEntities();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FVector& Velocity = Velocities[i].Value;
				FMassBoidsHealthFragment& HealthInfo = HealthInfos[i];
				FDronePendingExplosionFragment& PendingInfo = PendingInfos[i];

				if (PendingInfo.bPendingExplode == false)
				{
					continue;
				}

				if (CurrentFrame < PendingInfo.ExplodeAtFrame)
				{
					continue;
				}

				PendingInfo.bPendingExplode = false;
				PendingInfo.ExplodeAtFrame = 0;

				if (HealthInfo.Health <= 0.0f)
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