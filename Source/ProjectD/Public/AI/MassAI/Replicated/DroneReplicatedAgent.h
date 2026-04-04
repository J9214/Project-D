#pragma once

#include "CoreMinimal.h"
#include "MassReplicationTypes.h"
#include "MassReplicationTransformHandlers.h"
#include "AI/MassAI/MassEntityCueId.h"
#include "DroneReplicatedAgent.generated.h"

USTRUCT()
struct FDroneReplicatedAgent : public FReplicatedAgentBase
{
	GENERATED_BODY()

public:
	void SetFromTransform(const FTransform& Transform);
	void SetDead(const FVector& InDeathLocation, const EMassEntityCueId InCueId);

public:
	FORCEINLINE const FReplicatedAgentPositionYawData& GetPositionYaw() const { return PositionYaw; };
	FORCEINLINE FReplicatedAgentPositionYawData& GetPositionYawMutable() { return PositionYaw; };
	FORCEINLINE FVector GetPosition() const { return PositionYaw.GetPosition(); };
	FORCEINLINE float GetYawRadians() const { return PositionYaw.GetYaw(); };

	FORCEINLINE bool GetIsDead() const { return (bDead == true); }
	FORCEINLINE FVector GetDeathLocation() const { return DeathLocation; }
	FORCEINLINE EMassEntityCueId GetCueId() const { return CueId; }

private:
	UPROPERTY(Transient)
	FReplicatedAgentPositionYawData PositionYaw;

	UPROPERTY(Transient)
	bool bDead = false;

	UPROPERTY(Transient)
	FVector_NetQuantize DeathLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	EMassEntityCueId CueId = EMassEntityCueId::None;
};