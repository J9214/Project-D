#include "AI/MassAI/DroneSpawner.h"
#include "AI/MassAI/CheckDroneAllExplodeSubsystem.h"

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

void ADroneSpawner::ExploseAllDrones()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	UCheckDroneAllExplodeSubsystem* AllExplodeSubsystem = World->GetSubsystem<UCheckDroneAllExplodeSubsystem>();
	if (IsValid(AllExplodeSubsystem) == false)
	{
		return;
	}

	AllExplodeSubsystem->RequestExplodeAllDrones();
}