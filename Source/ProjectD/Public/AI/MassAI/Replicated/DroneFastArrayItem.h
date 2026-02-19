#pragma once

#include "CoreMinimal.h"
#include "MassReplicationTypes.h"
#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"
#include "DroneFastArrayItem.generated.h"

USTRUCT()
struct FDroneFastArrayItem : public FMassFastArrayItemBase
{
	GENERATED_BODY()

public:
	typedef FDroneReplicatedAgent FReplicatedAgentType;

	FDroneFastArrayItem();
	FDroneFastArrayItem(const FDroneReplicatedAgent& InAgent, const FMassReplicatedAgentHandle InHandle);

public:
	FORCEINLINE const FDroneReplicatedAgent& GetAgent() const { return Agent; }
	FORCEINLINE FDroneReplicatedAgent& GetAgentMutable() { return Agent; }

public:
	UPROPERTY()
	FDroneReplicatedAgent Agent;
};