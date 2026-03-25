#pragma once

#include "CoreMinimal.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "SimpleCircleSpawnPointsGenerator.generated.h"

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECTD_API USimpleCircleSpawnPointsGenerator : public UMassEntitySpawnDataGeneratorBase
{
	GENERATED_BODY()

public:
	virtual void Generate(
		UObject& QueryOwner,
		TConstArrayView<FMassSpawnedEntityType> EntityTypes,
		int32 Count,
		FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate
	) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float MinRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float MaxRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float HeightOffset = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bFaceCenter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bRandomYawWhenNotFacingCenter = true;
};