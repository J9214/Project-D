#include "AI/MassAI/Replicated/DroneReplicationBootingSubsystem.h"
#include "MassReplicationSubsystem.h"
#include "MassClientBubbleInfoBase.h"
#include "AI/MassAI/Replicated/DroneClientBubbleInfo.h"
#include "ProjectD/ProjectD.h"

void UDroneReplicationBootingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UMassReplicationSubsystem>();

	if (ShouldRunForThisWorld() == false)
	{
		return;
	}

	if (bRegisteredBubbleClass == true)
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
	UE_LOG(LogProjectD, Warning, TEXT("[DroneRep][Bootstrap] PID=%d World=%s NetMode=%d"),
		FPlatformProcess::GetCurrentProcessId(),
		*GetNameSafe(GetWorld()),
		(int32)GetWorld()->GetNetMode());

	bRegisteredBubbleClass = true;
}

void UDroneReplicationBootingSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UDroneReplicationBootingSubsystem::ShouldRunForThisWorld() const
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
	if (NetMode == NM_Client)
	{
		return false;
	}

	return true;
}
