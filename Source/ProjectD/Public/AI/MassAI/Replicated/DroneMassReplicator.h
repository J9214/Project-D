#pragma once

#include "CoreMinimal.h"
#include "MassReplicationProcessor.h"
#include "DroneMassReplicator.generated.h"

struct FMassEntityQuery;
struct FMassExecutionContext;
struct FMassReplicationContext;

UCLASS()
class PROJECTD_API UDroneMassReplicator : public UMassReplicatorBase
{
	GENERATED_BODY()

public:
	virtual void AddRequirements(FMassEntityQuery& EntityQuery) override;
	virtual void ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext) override;
};