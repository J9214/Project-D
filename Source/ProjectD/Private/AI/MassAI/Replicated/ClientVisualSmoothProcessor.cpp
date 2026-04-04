#include "AI/MassAI/Replicated/ClientVisualSmoothProcessor.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"

UClientVisualSmoothProcessor::UClientVisualSmoothProcessor()
	:EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::Client;
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	bAutoRegisterWithProcessingPhases = true;
}

void UClientVisualSmoothProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FClientVisualFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);
}

void UClientVisualSmoothProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaSeconds = Context.GetDeltaTimeSeconds();

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaSeconds](FMassExecutionContext& ChunkContext)
		{
			const int32 NumEntities = ChunkContext.GetNumEntities();

			TArrayView<FTransformFragment> TransformList = ChunkContext.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FClientVisualFragment> VisualTargetList = ChunkContext.GetMutableFragmentView<FClientVisualFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FTransform& Transform = TransformList[i].GetMutableTransform();
				FClientVisualFragment& VisualTarget = VisualTargetList[i];

				if (VisualTarget.bInitialized == false)
				{
					continue;
				}

				if (VisualTarget.bDead == true)
				{
					const FVector UseLoc =
						(VisualTarget.DeathLocation.IsNearlyZero() == false)
						? VisualTarget.DeathLocation
						: VisualTarget.TargetLocation;

					Transform.SetLocation(UseLoc);
					Transform.SetScale3D(FVector::ZeroVector);

					VisualTarget.bSnapThisFrame = false;
					continue;
				}

				Transform.SetScale3D(FVector::OneVector);

				if (VisualTarget.bSnapThisFrame == true)
				{
					Transform.SetLocation(VisualTarget.TargetLocation);
					Transform.SetRotation(VisualTarget.TargetRotation);

					VisualTarget.bSnapThisFrame = false;
					continue;
				}

				const FVector CurrentLocation = Transform.GetLocation();
				const FQuat CurrentRotation = Transform.GetRotation();

				const FVector NewLocation = FMath::VInterpTo(
					CurrentLocation,
					VisualTarget.TargetLocation,
					DeltaSeconds,
					LocationInterpSpeed
				);

				const FQuat NewRotation = FMath::QInterpTo(
					CurrentRotation,
					VisualTarget.TargetRotation,
					DeltaSeconds,
					RotationInterpSpeed
				);

				Transform.SetLocation(NewLocation);
				Transform.SetRotation(NewRotation);
			}
		});

}
