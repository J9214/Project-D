#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Structs/SpawnPlacementData.h"
#include "PDSpawnSkillTableRow.generated.h"

USTRUCT(BlueprintType)
struct PROJECTD_API FPDSpawnSkillTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Skill")
	FSpawnPlacementData PlacementData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Skill")
	FText DisplayName;
};
