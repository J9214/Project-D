#include "AI/MassAI/DroneSpawner.h"

ADroneSpawner::ADroneSpawner()
{
	bReplicates = false;
	SetReplicateMovement(false);
	bAlwaysRelevant = false;
	bNetLoadOnClient = false;
}

void ADroneSpawner::SpawnDrones()
{
	if (HasAuthority() == false)
	{
		return;
	}

	DoSpawning();
}