#include "Subsystem/PlayerLocSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void UPlayerLocSubsystem::GetPlayerControllers(TArray<class APlayerController*>& OutControllers) const
{
	OutControllers.Reset();

	const UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (IsValid(PC) == true)
		{
			OutControllers.Add(PC);
		}
	}
}

void UPlayerLocSubsystem::GetPlayerPawns(TArray<class APawn*>& OutPawns) const
{
	OutPawns.Reset();

	TArray<APlayerController*> Controllers;
	GetPlayerControllers(Controllers);

	OutPawns.Reserve(Controllers.Num());

	for (APlayerController* PC : Controllers)
	{
		if (IsValid(PC) == false)
		{
			continue;
		}

		APawn* Pawn = PC->GetPawn();
		if (IsValid(Pawn) == true)
		{
			OutPawns.Add(Pawn);
		}
	}
}

void UPlayerLocSubsystem::GetPlayerLocations(TArray<FVector>& OutLocations) const
{
	OutLocations.Reset();

	TArray<APawn*> Pawns;
	GetPlayerPawns(Pawns);

	OutLocations.Reserve(Pawns.Num());

	for (APawn* Pawn : Pawns)
	{
		OutLocations.Add(Pawn->GetActorLocation());
	}
}

void UPlayerLocSubsystem::GetPlayerViewpoints(TArray<FVector>& OutOrigins, TArray<FVector>& OutDirs) const
{
	OutOrigins.Reset();
	OutDirs.Reset();

	TArray<APlayerController*> Controllers;
	GetPlayerControllers(Controllers);

	OutOrigins.Reserve(Controllers.Num());
	OutDirs.Reserve(Controllers.Num());

	for (APlayerController* PC : Controllers)
	{
		if (IsValid(PC) == false)
		{
			continue;
		}

		FVector ViewLoc = FVector::ZeroVector;
		FRotator ViewRot = FRotator::ZeroRotator;
		PC->GetPlayerViewPoint(ViewLoc, ViewRot);

		OutOrigins.Add(ViewLoc);
		OutDirs.Add(ViewRot.Vector().GetSafeNormal());
	}
}
