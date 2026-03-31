#include "Object/GoalPost.h"
#include "Object/BallCore.h"
#include "GameFramework/PlayerController.h"
#include "Pawn/PDPawnBase.h" 
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "GameState/PDGameStateBase.h"
#include "PlayerState/PDPlayerState.h"
#include "ProjectD/ProjectD.h"
#include "GameMode/PDGameModeBase.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/Ingame/ObjectInfo.h"
#include "UI/PDTeamColorFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

namespace
{
ETeamType ResolveLocalViewerTeamID(const AActor* ContextActor)
{
	if (IsValid(ContextActor) == false)
	{
		return ETeamType::None;
	}

	const UWorld* World = ContextActor->GetWorld();
	if (World == nullptr)
	{
		return ETeamType::None;
	}

	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	const APDPlayerState* LocalPlayerState = IsValid(PlayerController) ? PlayerController->GetPlayerState<APDPlayerState>() : nullptr;
	return IsValid(LocalPlayerState) ? LocalPlayerState->GetTeamID() : ETeamType::None;
}
}

AGoalPost::AGoalPost()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	GoalHoldTime = 10.0f;
	GoalHoldTimeUpdateInterval = 0.5f;
	RemainingHoldTime = 0.0f;

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

	UpdateInfoWidgetColor();
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

	UpdateInfoWidgetColor();
}

void AGoalPost::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoalPost, RemainingHoldTime);
	DOREPLIFETIME(AGoalPost, ObjectInfoTeamID);
}

void AGoalPost::ResetGoalPost()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] ResetGoalPost failed. World is nullptr."));
		return;
	}

	World->GetTimerManager().ClearTimer(HoldTimer);
	World->GetTimerManager().ClearTimer(HoldTimeUpdateTimer);

	PlacedBall = nullptr;
	SetObjectInfoTeamID(ETeamType::None);
	SetRemainingHoldTime(GoalHoldTime);
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
	
	Ball->PlaceIntoGoal(StaticMesh);
	
	if (APDGameStateBase* GS = GetWorld()->GetGameState<APDGameStateBase>())
	{
		APDPlayerState* PS = Pawn->GetPlayerState<APDPlayerState>();
		if (IsValid(PS) == true)
		{
			SetObjectInfoTeamID(PS->GetTeamID());
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

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] StealBall failed. World is nullptr."));
		return;
	}

	World->GetTimerManager().ClearTimer(HoldTimer);
	World->GetTimerManager().ClearTimer(HoldTimeUpdateTimer);
	SetObjectInfoTeamID(ETeamType::None);
	SetRemainingHoldTime(GoalHoldTime);

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
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] StartHoldTimer failed. World is nullptr."));
		return;
	}

	if ((GoalHoldTime > 0.0f) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] StartHoldTimer failed. GoalHoldTime must be greater than zero. GoalHoldTime=%.2f"), GoalHoldTime);
		SetRemainingHoldTime(GoalHoldTime);
		return;
	}

	World->GetTimerManager().ClearTimer(HoldTimer);
	World->GetTimerManager().ClearTimer(HoldTimeUpdateTimer);
	SetRemainingHoldTime(0.0f);

	World->GetTimerManager().SetTimer(HoldTimer, this, &AGoalPost::OnHoldComplete, GoalHoldTime, false);
	World->GetTimerManager().SetTimer(HoldTimeUpdateTimer, this, &AGoalPost::UpdateRemainingHoldTime, GoalHoldTimeUpdateInterval, true);
}

void AGoalPost::UpdateRemainingHoldTime()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] UpdateRemainingHoldTime failed. World is nullptr."));
		return;
	}

	float NewRemainingHoldTime = 0.0f;
	const bool bIsHoldTimerActive = World->GetTimerManager().IsTimerActive(HoldTimer);
	if (bIsHoldTimerActive == true)
	{
		NewRemainingHoldTime = FMath::Max(GoalHoldTime - World->GetTimerManager().GetTimerRemaining(HoldTimer), 0.0f);
	}

	SetRemainingHoldTime(NewRemainingHoldTime);
}

void AGoalPost::OnHoldComplete()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] OnHoldComplete failed. World is nullptr."));
		return;
	}

	World->GetTimerManager().ClearTimer(HoldTimeUpdateTimer);
	SetRemainingHoldTime(GoalHoldTime);

	if (IsValid(PlacedBall) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] OnHoldComplete skipped. PlacedBall is invalid."));
		return;
	}

	APDGameModeBase* GM = World->GetAuthGameMode<APDGameModeBase>();
	if (IsValid(GM) == true)
	{
		GM->HandleGoalScored(this, PlacedBall);
	}
	else
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GoalPost] OnHoldComplete warning. GameMode is invalid."));
	}

	PlacedBall = nullptr;
}

void AGoalPost::SetRemainingHoldTime(float InRemainingHoldTime)
{
	const float ClampedRemainingHoldTime = FMath::Max(InRemainingHoldTime, 0.0f);

	if (FMath::IsNearlyEqual(RemainingHoldTime, ClampedRemainingHoldTime, KINDA_SMALL_NUMBER) == true)
	{
		return;
	}

	RemainingHoldTime = ClampedRemainingHoldTime;
	HandleGoalHoldRemainingTimeChanged(RemainingHoldTime);
}

void AGoalPost::HandleGoalHoldRemainingTimeChanged(float InRemainingHoldTime)
{
	if (IsValid(CachedInfoWidget) == false)
	{
		return;
	}

	float FillAmount = 0.0f;
	if ((GoalHoldTime > 0.0f) == true)
	{
		FillAmount = FMath::Clamp(InRemainingHoldTime / GoalHoldTime, 0.0f, 1.0f);
	}

	CachedInfoWidget->SetFillAmount(FillAmount);
}

void AGoalPost::OnRep_RemainingHoldTime()
{
	HandleGoalHoldRemainingTimeChanged(RemainingHoldTime);
}

void AGoalPost::OnRep_ObjectInfoTeamID()
{
	UpdateInfoWidgetColor();
}

void AGoalPost::UpdateInfoWidgetColor()
{
	if (IsValid(CachedInfoWidget) == false)
	{
		return;
	}

	const ETeamType ViewerTeamID = ResolveLocalViewerTeamID(this);
	if (LastAppliedViewerTeamID == ViewerTeamID && LastAppliedObjectInfoTeamID == ObjectInfoTeamID)
	{
		return;
	}

	CachedInfoWidget->SetInterfaceColor(UPDTeamColorFunctionLibrary::GetRelativeTeamColor(ViewerTeamID, ObjectInfoTeamID));
	LastAppliedViewerTeamID = ViewerTeamID;
	LastAppliedObjectInfoTeamID = ObjectInfoTeamID;
}

void AGoalPost::SetObjectInfoTeamID(const ETeamType NewTeamID)
{
	if (ObjectInfoTeamID == NewTeamID)
	{
		return;
	}

	ObjectInfoTeamID = NewTeamID;
	UpdateInfoWidgetColor();
}
