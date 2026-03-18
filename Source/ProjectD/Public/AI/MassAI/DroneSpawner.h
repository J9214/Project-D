#pragma once

#include "CoreMinimal.h"
#include "MassSpawner.h"
#include "DroneSpawner.generated.h"

UCLASS()
class PROJECTD_API ADroneSpawner : public AMassSpawner
{
	GENERATED_BODY()

public:
	ADroneSpawner();
	
public:
	void SpawnDrones();

	void ExploseAllDrones();
};
