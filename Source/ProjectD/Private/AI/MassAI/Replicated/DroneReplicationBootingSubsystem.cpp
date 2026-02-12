#include "AI/MassAI/Replicated/DroneReplicationBootingSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "MassReplicationSubsystem.h"
#include "AI/MassAI/Replicated/DroneClientBubbleInfo.h"
#include "MassVisualizer.h"
#include "ProjectD/ProjectD.h"

void UDroneReplicationBootingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UMassReplicationSubsystem>();

	RegisterBubbleInfoClassIfNeeded();
	EnsureMassVisualizerActorIfNeeded();
}

void UDroneReplicationBootingSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UDroneReplicationBootingSubsystem::ShouldRunForServer() const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	if (World->IsGameWorld() == false)
	{
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_Client ||
		NetMode == NM_Standalone)
	{
		return false;
	}

	return true;
}

bool UDroneReplicationBootingSubsystem::ShouldRunForClient() const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	if (World->IsGameWorld() == false)
	{
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_DedicatedServer ||
		NetMode == NM_ListenServer)
	{
		return false;
	}

	return true;
}

void UDroneReplicationBootingSubsystem::RegisterBubbleInfoClassIfNeeded()
{
	if (bRegisteredBubbleClass == true)
	{
		return;
	}

	if (ShouldRunForServer() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UMassReplicationSubsystem* RepSubsystem = World->GetSubsystem<UMassReplicationSubsystem>();
	if (RepSubsystem == nullptr)
	{
		return;
	}

	const TSubclassOf<AMassClientBubbleInfoBase> BubbleClass = ADroneClientBubbleInfo::StaticClass();
	const FMassBubbleInfoClassHandle Handle = RepSubsystem->RegisterBubbleInfoClass(BubbleClass);

	const bool bValid = RepSubsystem->IsBubbleClassHandleValid(Handle);

	UE_LOG(LogProjectD, Warning, TEXT("[DroneRep][Bootstrap] RegisterBubbleInfoClass PID=%d World=%s NetMode=%d HandleValid=%d"),
		FPlatformProcess::GetCurrentProcessId(),
		*GetNameSafe(World),
		(int32)World->GetNetMode(),
		(bValid == true) ? 1 : 0);

	bRegisteredBubbleClass = true;
}


void UDroneReplicationBootingSubsystem::EnsureMassVisualizerActorIfNeeded()
{
	if (bSpawnedMassVisualizer == true)
	{
		return;
	}

	if (ShouldRunForClient() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<AMassVisualizer> It(World); It; ++It)
	{
		bSpawnedMassVisualizer = true;
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMassVisualizer* Spawned = World->SpawnActor<AMassVisualizer>(AMassVisualizer::StaticClass(), FTransform::Identity, Params);

	UE_LOG(LogProjectD, Warning, TEXT("[DroneRep][Bootstrap] MassVisualizer Spawned=%d World=%s NetMode=%d PID=%d"),
		(Spawned != nullptr) ? 1 : 0,
		*GetNameSafe(World),
		(int32)World->GetNetMode(),
		FPlatformProcess::GetCurrentProcessId());

	bSpawnedMassVisualizer = true;
}
