#pragma once

#include "CoreMinimal.h"
#include "MassReplicationTypes.h"
#include "MassReplicationTransformHandlers.h"
#include "DroneReplicatedAgent.generated.h"

USTRUCT()
struct FDroneReplicatedAgent : public FReplicatedAgentBase
{
	GENERATED_BODY()

public:
	void SetFromTransform(const FTransform& Transform);

public:
	FORCEINLINE const FReplicatedAgentPositionYawData& GetPositionYaw() const { return PositionYaw; };
	FORCEINLINE FReplicatedAgentPositionYawData& GetPositionYawMutable() { return PositionYaw; };
	FORCEINLINE FVector GetPosition() const { return PositionYaw.GetPosition(); };
	FORCEINLINE float GetYawRadians() const { return PositionYaw.GetYaw(); };

private:
	UPROPERTY(Transient)
	FReplicatedAgentPositionYawData PositionYaw;
};