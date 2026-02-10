#include "AI/MassAI/Replicated/DroneReplicatedAgent.h"

void FDroneReplicatedAgent::SetFromTransform(const FTransform& Transform)
{
	PositionYaw.SetPosition(Transform.GetLocation());

	const float YawRad = FMath::DegreesToRadians(Transform.Rotator().Yaw);
	PositionYaw.SetYaw(YawRad);
}