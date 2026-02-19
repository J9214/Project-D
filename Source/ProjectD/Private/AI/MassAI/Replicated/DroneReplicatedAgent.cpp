#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"

void FDroneReplicatedAgent::SetFromTransform(const FTransform& Transform)
{
	PositionYaw.SetPosition(Transform.GetLocation());

	const float YawRad = FMath::DegreesToRadians(Transform.Rotator().Yaw);
	PositionYaw.SetYaw(YawRad);
}

void FDroneReplicatedAgent::SetDead(const FVector& InDeathLocation, const EMassEntityCueId InCueId)
{
	bDead = true;
	DeathLocation = InDeathLocation;
	DeathCueId = InCueId;
}