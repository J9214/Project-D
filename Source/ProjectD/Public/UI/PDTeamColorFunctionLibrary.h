#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interface/PDTeamInterface.h"
#include "PDTeamColorFunctionLibrary.generated.h"

UCLASS()
class PROJECTD_API UPDTeamColorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "UI|Team")
	static FLinearColor GetRelativeTeamColor(ETeamType LocalTeamID, ETeamType TargetTeamID);

	UFUNCTION(BlueprintPure, Category = "UI|Team")
	static FSlateColor GetRelativeTeamSlateColor(ETeamType LocalTeamID, ETeamType TargetTeamID);

	UFUNCTION(BlueprintPure, Category = "UI|Team")
	static ETeamType GetTeamTypeFromIndex(int32 TeamIndex);

	UFUNCTION(BlueprintPure, Category = "UI|Team")
	static FLinearColor GetRelativeTeamColorByIndex(ETeamType LocalTeamID, int32 TargetTeamIndex);

	UFUNCTION(BlueprintPure, Category = "UI|Team")
	static FSlateColor GetRelativeTeamSlateColorByIndex(ETeamType LocalTeamID, int32 TargetTeamIndex);

private:
	static bool IsPlayableTeam(ETeamType TeamID);
};
