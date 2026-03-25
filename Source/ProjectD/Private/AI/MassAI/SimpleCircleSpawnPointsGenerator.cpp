#include "AI/MassAI/SimpleCircleSpawnPointsGenerator.h"
#include "MassSpawner.h"
#include "MassSpawnerTypes.h"
#include "MassSpawnLocationProcessor.h"
#include "ProjectD/ProjectD.h"

void USimpleCircleSpawnPointsGenerator::Generate(
	UObject& QueryOwner,
	TConstArrayView<FMassSpawnedEntityType> EntityTypes,
	int32 Count,
	FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate
) const
{
	TArray<FMassEntitySpawnDataGeneratorResult> Results;

	if (Count <= 0)
	{
		FinishedGeneratingSpawnPointsDelegate.ExecuteIfBound(Results);
		return;
	}

	if (EntityTypes.Num() <= 0)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[SimpleCircleSpawnPointsGenerator] Generate failed. EntityTypes is empty."));
		FinishedGeneratingSpawnPointsDelegate.ExecuteIfBound(Results);
		return;
	}

	AMassSpawner* MassSpawner = Cast<AMassSpawner>(&QueryOwner);
	if (IsValid(MassSpawner) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[SimpleCircleSpawnPointsGenerator] Generate failed. QueryOwner is not a valid AMassSpawner."));
		FinishedGeneratingSpawnPointsDelegate.ExecuteIfBound(Results);
		return;
	}

	const FVector SpawnOrigin = MassSpawner->GetActorLocation();

	BuildResultsFromEntityTypes(Count, EntityTypes, Results);

	if (Results.Num() <= 0)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[SimpleCircleSpawnPointsGenerator] Generate failed. Results is empty after BuildResultsFromEntityTypes."));
		FinishedGeneratingSpawnPointsDelegate.ExecuteIfBound(Results);
		return;
	}

	FRandomStream RandomStream;
	RandomStream.Initialize(GetRandomSelectionSeed());

	for (FMassEntitySpawnDataGeneratorResult& Result : Results)
	{
		TArray<FTransform> SpawnTransforms;
		SpawnTransforms.Reserve(Result.NumEntities);

		for (int32 i = 0; i < Result.NumEntities; ++i)
		{
			const float RandomAngleDeg = RandomStream.FRandRange(0.0f, 360.0f);
			const float RandomAngleRad = FMath::DegreesToRadians(RandomAngleDeg);
			const float RandomRadius = RandomStream.FRandRange(MinRadius, MaxRadius);

			const FVector Direction = FVector(FMath::Cos(RandomAngleRad), FMath::Sin(RandomAngleRad), 0.0f);
			const FVector SpawnLocation = SpawnOrigin + (Direction * RandomRadius) + FVector(0.0f, 0.0f, HeightOffset);

			FRotator SpawnRotation = FRotator::ZeroRotator;

			if (bFaceCenter == true)
			{
				SpawnRotation = (SpawnOrigin - SpawnLocation).Rotation();
			}
			else
			{
				if (bRandomYawWhenNotFacingCenter == true)
				{
					SpawnRotation = FRotator(0.0f, RandomAngleDeg, 0.0f);
				}
			}

			SpawnTransforms.Add(FTransform(SpawnRotation, SpawnLocation, FVector::OneVector));
		}

		Result.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
		Result.SpawnData.InitializeAs<FMassTransformsSpawnData>();

		FMassTransformsSpawnData& TransformSpawnData = Result.SpawnData.GetMutable<FMassTransformsSpawnData>();
		TransformSpawnData.Transforms = MoveTemp(SpawnTransforms);
	}

	UE_LOG(
		LogProjectD,
		Log,
		TEXT("[SimpleCircleSpawnPointsGenerator] Generate completed. SpawnOrigin=(%.2f, %.2f, %.2f), Count=%d, ResultNum=%d"),
		SpawnOrigin.X,
		SpawnOrigin.Y,
		SpawnOrigin.Z,
		Count,
		Results.Num()
	);

	FinishedGeneratingSpawnPointsDelegate.ExecuteIfBound(Results);
}