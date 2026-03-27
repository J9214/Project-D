#include "Object/GoalPost.h"
#include "Object/BallCore.h"
#include "Pawn/PDPawnBase.h" 
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "GameState/PDGameStateBase.h"
#include "PlayerState/PDPlayerState.h"
#include "ProjectD/ProjectD.h"
#include "GameMode/PDGameModeBase.h"
#include "Components/WidgetComponent.h"
#include "UI/Ingame/ObjectInfo.h"
#include "Kismet/GameplayStatics.h"

AGoalPost::AGoalPost()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	GoalHoldTime = 10.0f;

	GoalWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("GoalWidget"));
	if (GetRootComponent())
	{
		GoalWidget->SetupAttachment(StaticMesh);
	}

	GoalWidget->SetWidgetSpace(EWidgetSpace::Screen);
	GoalWidget->SetDrawAtDesiredSize(true);
}

void AGoalPost::BeginPlay()
{
	Super::BeginPlay();

	if (GoalWidget)
	{
		CachedInfoWidget = Cast<UObjectInfo>(GoalWidget->GetUserWidgetObject());

		if (!CachedInfoWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("GoalWidget's UserWidget is not UObjectInfo!"));
		}
	}
}

void AGoalPost::OnInteract_Implementation(AActor* Interactor)
{
	APDPawnBase* PDPawn = Cast<APDPawnBase>(Interactor);
	if (!PDPawn)
	{
		return;
	}

	ABallCore* Ball = Cast<ABallCore>(PDPawn->GetCarriedObject());
	if (IsValid(Ball))
	{
		PlaceBall(PDPawn, Ball);
	}
	else if (PlacedBall)
	{
		StealBall(PDPawn);
	}
}

void AGoalPost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CachedInfoWidget)
	{
		return;
	}

	if (!CachedPlayer)
	{
		CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	if (CachedPlayer)
	{
		float Dist = GetDistanceTo(CachedPlayer);
		CachedInfoWidget->UpdateDistanceUI((int32)(Dist / 100.0f));
	}
}

void AGoalPost::ResetGoalPost()
{
	if (GetWorld() == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] ResetGoalPost failed. World is nullptr."));
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(HoldTimer);

	PlacedBall = nullptr;
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

	if (APDPawnBase* PD = Cast<APDPawnBase>(Pawn))
	{
		PD->Server_ForceClearCarriedBall();
	}
	
	Ball->PlaceIntoGoal(RootComponent);
	
	if (APDGameStateBase* GS = GetWorld()->GetGameState<APDGameStateBase>())
	{
		APDPlayerState* PS = Pawn->GetPlayerState<APDPlayerState>();
		if (IsValid(PS) == true)
		{
			GS->SetGoalInstigator(PS);
		}
	}

	if (APDGameModeBase* GM = GetWorld()->GetAuthGameMode<APDGameModeBase>())
	{
		GM->HandleGoalEntered(this, Ball);
	}

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

	if (!IsValid(BallToSteal))
	{
		return;
	}
	
	BallToSteal->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	BallToSteal->SetPlacedInGoal(false);

	if (APDPawnBase* PD = Cast<APDPawnBase>(Stealer))
	{
		PD->Server_PickUpObject(BallToSteal);
	}
}

void AGoalPost::StartHoldTimer()
{
	GetWorld()->GetTimerManager().SetTimer(HoldTimer, this, &AGoalPost::OnHoldComplete, GoalHoldTime, false);
}

void AGoalPost::OnHoldComplete()
{
	if (IsValid(PlacedBall) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] OnHoldComplete skipped. PlacedBall is invalid."));
		return;
	}

	if (APDGameModeBase* GM = GetWorld()->GetAuthGameMode<APDGameModeBase>())
	{
		GM->HandleGoalScored(this, PlacedBall);
	}

	PlacedBall = nullptr;
}