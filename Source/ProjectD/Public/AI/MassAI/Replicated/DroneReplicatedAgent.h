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
	void SetDead(const FVector& InDeathLocation, const uint8 InCueId);

public:
	FORCEINLINE const FReplicatedAgentPositionYawData& GetPositionYaw() const { return PositionYaw; };
	FORCEINLINE FReplicatedAgentPositionYawData& GetPositionYawMutable() { return PositionYaw; };
	FORCEINLINE FVector GetPosition() const { return PositionYaw.GetPosition(); };
	FORCEINLINE float GetYawRadians() const { return PositionYaw.GetYaw(); };

	FORCEINLINE bool GetIsDead() const { return (bDead == 1); }
	FORCEINLINE FVector GetDeathLocation() const { return DeathLocation; }
	FORCEINLINE uint8 GetDeathCueId() const { return DeathCueId; }

private:
	UPROPERTY(Transient)
	FReplicatedAgentPositionYawData PositionYaw;

	UPROPERTY()
	bool bDead = 0;

	UPROPERTY()
	FVector_NetQuantize DeathLocation = FVector::ZeroVector;

	UPROPERTY()
	uint8 DeathCueId = 0;
};