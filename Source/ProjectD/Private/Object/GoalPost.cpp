#include "Object/GoalPost.h"
#include "Object/BallCore.h"
#include "Pawn/PDPawnBase.h" 
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "GameState/PDGameStateBase.h"

AGoalPost::AGoalPost()
{
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void AGoalPost::OnInteract_Implementation(AActor* Interactor)
{
	APDPawnBase* PDPawn = Cast<APDPawnBase>(Interactor);
	if (!PDPawn)
	{
		return;
	}

	if (PDPawn->GetCarriedBall())
	{
		PlaceBall(PDPawn, PDPawn->GetCarriedBall());
	}
	else if (PlacedBall)
	{
		StealBall(PDPawn);
	}
}

bool AGoalPost::CanPlaceBall(APawn* Pawn, ABallCore* Ball) const
{
	return Pawn && Ball && !PlacedBall;
}

void AGoalPost::PlaceBall(APawn* Pawn, ABallCore* Ball)
{
	if (!HasAuthority())
	{
		return;
	}
	if (!CanPlaceBall(Pawn, Ball)) 
	{
		return;
	}

	PlacedBall = Ball;

	Ball->ClearCarrier();

	if (APDPawnBase* PD = Cast<APDPawnBase>(Pawn))
	{
		PD->Server_ForceClearCarriedBall();
	}

	Ball->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	StartHoldTimer();
}

void AGoalPost::StealBall(APawn* Stealer)
{
	if (!HasAuthority()) 
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(HoldTimer);

	ABallCore* BallToSteal = PlacedBall;

	PlacedBall = nullptr;

	if (APDPawnBase* PD = Cast<APDPawnBase>(Stealer))
	{
		PD->Server_PickUpBall(BallToSteal);
	}
}

void AGoalPost::StartHoldTimer()
{
	GetWorld()->GetTimerManager().SetTimer(HoldTimer, this, &AGoalPost::OnHoldComplete, 10.f, false);
}

void AGoalPost::OnHoldComplete()
{
	if (PlacedBall)
	{
		PlacedBall->Destroy();
		PlacedBall = nullptr;
	}

	APDGameStateBase* GS = GetWorld()->GetGameState<APDGameStateBase>();

	if (GS)
	{
		GS->GoalScored();
	}
}