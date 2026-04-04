#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CheckDroneAllExplodeSubsystem.generated.h"

UCLASS()
class PROJECTD_API UCheckDroneAllExplodeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	void RequestExplodeAllDrones();
	bool ConsumeExplodeAllRequest();

private:
	bool bExplodeAllRequested = false;
};
