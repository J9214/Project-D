#include "GameMode/PDGameModeBase.h"
#include "PlayerState/PDPlayerState.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "GameState/PDGameStateBase.h"
#include "Controller/PDPlayerController.h"
#include "AI/MassAI/MassProxyPoolSubsystem.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Object/BallCore.h"
#include "Object/GoalPost.h"
#include "ProjectD/ProjectD.h"

APDGameModeBase::APDGameModeBase()
{
    TotalGameDurationSec = 1800;

    TargetScoreToWin = 1000;

    NextRoundDelaySec = 3.0f;

    TeamRespawnRadiusFromBall = 900.0f;
    RespawnHeightOffset = 50.0f;

    CurrentRoundIndex = 0;
    RoundPhase = ERoundPhase::Waiting;
    CurrentRoundBallSpawnLocation = FVector::ZeroVector;

    CachedBallCore = nullptr;
    BallCoreClass = nullptr;
}

void APDGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    const UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(LogProjectD, Error, TEXT("[GameMode] BeginPlay failed. World is nullptr."));
        return;
    }

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] BeginPlay. Map=%s GM=%s DefaultPawnClass=%s"),
        *World->GetMapName(),
        *GetNameSafe(GetClass()),
        *GetNameSafe(DefaultPawnClass)
    );

    UMassProxyPoolSubsystem* Pool = World->GetSubsystem<UMassProxyPoolSubsystem>();
    if (IsValid(Pool) == true)
    {
        const int32 PoolSize = 512;
        Pool->InitPool(PoolSize);
    }
    else
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] MassProxyPoolSubsystem is invalid."));
    }

	StartRound();
    CacheRoundActors();
    StartMatchFlow();
}

void APDGameModeBase::PlayerDied(AController* Controller)
{
    if (IsValid(Controller) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PlayerDied failed. Controller is invalid."));
        return;
    }

    if (APDPlayerState* PS = Controller->GetPlayerState<APDPlayerState>())
    {
        PS->SetDeadState();
    }
    else
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PlayerDied. PlayerState is invalid."));
    }

    FTimerHandle TimerHandle;
    FTimerDelegate TimerDel;
    TimerDel.BindUObject(this, &APDGameModeBase::PlayerRespawn, Controller);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TimerHandle, TimerDel, 5.0f, false);
    }
    else
    {
        UE_LOG(LogProjectD, Error, TEXT("[GameMode] PlayerDied failed. World is invalid."));
    }
}

void APDGameModeBase::StartOvertime()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartOvertime failed. GameState is invalid."));
        return;
    }

    if (GS->bOvertime == true)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartOvertime skipped. Already overtime."));
        return;
    }

    GS->bOvertime = true;

    UE_LOG(LogProjectD, Log, TEXT("[GameMode] Overtime started."));
}

void APDGameModeBase::FinishGame(int32 WinnerTeamId)
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();

    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] FinishGame failed. GameState is invalid."));
        return;
    }

    if (GS->WinnerTeamId != INDEX_NONE)
    {
        UE_LOG(
            LogProjectD,
            Warning,
            TEXT("[GameMode] FinishGame skipped. Winner already decided. WinnerTeamId=%d"),
            GS->WinnerTeamId
        );
        return;
    }

    GS->WinnerTeamId = WinnerTeamId;
    GS->bOvertime = false;
    RoundPhase = ERoundPhase::GameEnded;

    GetWorldTimerManager().ClearTimer(GameTimerHandle);
    GetWorldTimerManager().ClearTimer(NextRoundTimerHandle);

}

void APDGameModeBase::HandleBallPickedUp(APDPlayerState* HolderPlayerState, ABallCore* Ball)
{
    if (RoundPhase != ERoundPhase::InRound)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleBallPickedUp skipped. RoundPhase is not InRound."));
        return;
    }

    if (IsValid(HolderPlayerState) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleBallPickedUp failed. HolderPlayerState is invalid."));
        return;
    }

    if (IsValid(Ball) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleBallPickedUp failed. Ball is invalid."));
        return;
    }


    TriggerDroneSpawnOnBallPickup(HolderPlayerState);
}

void APDGameModeBase::HandleGoalScored(AGoalPost* GoalPost, ABallCore* Ball)
{
    if (RoundPhase != ERoundPhase::InRound)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGoalScored skipped. RoundPhase is not InRound."));
        return;
    }

    if (IsValid(GoalPost) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGoalScored failed. GoalPost is invalid."));
        return;
    }

    if (IsValid(Ball) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGoalScored failed. Ball is invalid."));
        return;
    }

    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGoalScored failed. GameState is invalid."));
        return;
    }

    GS->GoalScored();

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] Goal scored. CurrentRoundIndex=%d"),
        CurrentRoundIndex
    );

    TriggerDroneExplosionOnGoal();

    if (GS->bOvertime == true)
    {
        bool bTie = false;
        const int32 BestTeamId = CalculateBestTeamId(bTie);

        if (bTie == false && BestTeamId != INDEX_NONE)
        {
            FinishGame(BestTeamId);
            return;
        }
    }

    if (TryFinishGameByScoreCondition() == true)
    {
        return;
    }

    RoundPhase = ERoundPhase::RoundEnded;
    PrepareNextRound();
}

void APDGameModeBase::PlayerRespawn(AController* Controller)
{
    if (IsValid(Controller) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PlayerRespawn failed. Controller is invalid."));
        return;
    }

    APawn* Pawn = Controller->GetPawn();
    if (IsValid(Pawn) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PlayerRespawn failed. Pawn is invalid."));
        return;
    }

    const FVector RespawnLocation = BuildRespawnLocationForController(Controller);
    if (RespawnLocation == FVector::ZeroVector)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PlayerRespawn failed. RespawnLocation is zero vector."));
        return;
    }

    const FRotator RespawnRotation = (CurrentRoundBallSpawnLocation - RespawnLocation).Rotation();
    Pawn->TeleportTo(RespawnLocation, RespawnRotation);

    if (APDPlayerState* PS = Controller->GetPlayerState<APDPlayerState>())
    {
        PS->SetReviveState();
    }
    else
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PlayerRespawn. PlayerState is invalid."));
    }
}

void APDGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (IsValid(NewPlayer) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PostLogin failed. NewPlayer is invalid."));
        return;
    }

    if (APDPlayerState* PS = NewPlayer->GetPlayerState<APDPlayerState>())
    {
        if (UPDAttributeSetBase* AS = PS->GetPDAttributeSetBase())
        {
            AS->OnOutOfHealth.AddUniqueDynamic(this, &APDGameModeBase::OnPlayerOutOfHealth);
        }
        else
        {
            UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PostLogin failed. AttributeSet is invalid."));
        }
    }
    else
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PostLogin failed. PlayerState is invalid."));
    }
}

void APDGameModeBase::OnPlayerOutOfHealth(AController* VictimController, AActor* DamageCauser)
{
    if (IsValid(VictimController) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] OnPlayerOutOfHealth failed. VictimController is invalid."));
        return;
    }

    PlayerDied(VictimController);
}

void APDGameModeBase::StartRound()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartRound failed. GameState is invalid."));
        return;
    }

    if (RoundPhase == ERoundPhase::GameEnded)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartRound skipped. RoundPhase is GameEnded."));
        return;
    }

    ResetRoundState();
    ResetPlacedGoalPostsForRound();
    ResetBallForRound();

    RoundPhase = ERoundPhase::InRound;
}

void APDGameModeBase::StartMatchFlow()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartMatchFlow failed. GameState is invalid."));
        return;
    }

    CurrentRoundIndex = 1;
    RoundPhase = ERoundPhase::Waiting;

    GS->RemainingTimeSec = TotalGameDurationSec;

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] Match start. TotalGameDurationSec=%d TargetScoreToWin=%d"),
        TotalGameDurationSec,
        TargetScoreToWin
    );

    GetWorldTimerManager().ClearTimer(GameTimerHandle);
    GetWorldTimerManager().SetTimer(
        GameTimerHandle,
        this,
        &APDGameModeBase::OnGameTick,
        1.0f,
        true
    );

    StartRound();
}

void APDGameModeBase::PrepareNextRound()
{
    if (RoundPhase == ERoundPhase::GameEnded)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PrepareNextRound skipped. Game already ended."));
        return;
    }

    CurrentRoundIndex++;

    GetWorldTimerManager().ClearTimer(NextRoundTimerHandle);
    GetWorldTimerManager().SetTimer(
        NextRoundTimerHandle,
        this,
        &APDGameModeBase::StartRound,
        NextRoundDelaySec,
        false
    );
}

void APDGameModeBase::OnGameTick()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] OnGameTick failed. GameState is invalid."));
        return;
    }

    if (RoundPhase == ERoundPhase::GameEnded)
    {
        return;
    }

    GS->RemainingTimeSec = FMath::Max(0, GS->RemainingTimeSec - 1);

    if (GS->RemainingTimeSec <= 0)
    {
        GetWorldTimerManager().ClearTimer(GameTimerHandle);
        HandleGameTimeExpired();
    }
}

void APDGameModeBase::HandleGameTimeExpired()
{
    UE_LOG(LogProjectD, Log, TEXT("[GameMode] HandleGameTimeExpired called."));

    if (RoundPhase == ERoundPhase::GameEnded)
    {
        return;
    }

    UE_LOG(LogProjectD, Log, TEXT("[GameMode] HandleGameTimeExpired called."));

    bool bTie = false;
    const int32 BestTeamId = CalculateBestTeamId(bTie);

    if (BestTeamId == INDEX_NONE)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGameTimeExpired failed. BestTeamId is INDEX_NONE."));
        return;
    }

    if (bTie == true)
    {
        UE_LOG(LogProjectD, Log, TEXT("[GameMode] Game time expired. Start overtime."));
        StartOvertime();
        return;
    }

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] Game time expired. Finish game. WinnerTeamId=%d"),
        BestTeamId
    );

    FinishGame(BestTeamId);
}

int32 APDGameModeBase::CalculateBestTeamId(bool& bOutTie) const
{
    bOutTie = false;

    const APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] CalculateBestTeamId failed. GameState is invalid."));
        return INDEX_NONE;
    }

    if (GS->TeamScores.Num() <= 0)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] CalculateBestTeamId failed. TeamScores is empty."));
        return INDEX_NONE;
    }

    int32 BestTeamId = INDEX_NONE;
    int32 BestScore = MIN_int32;

    for (int32 TeamId = 0; TeamId < GS->TeamScores.Num(); ++TeamId)
    {
        const int32 Score = GS->TeamScores[TeamId];

        if (Score > BestScore)
        {
            BestScore = Score;
            BestTeamId = TeamId;
            bOutTie = false;
        }
        else if (Score == BestScore)
        {
            bOutTie = true;
        }
    }

    return BestTeamId;
}

void APDGameModeBase::CacheRoundActors()
{
    CachePlacedGoalPosts();
    SpawnAndCacheBallCore();
}

void APDGameModeBase::CachePlacedGoalPosts()
{
    CachedGoalPosts.Empty();

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] CachePlacedGoalPosts failed. World is nullptr."));
        return;
    }

    for (TActorIterator<AGoalPost> It(World); It; ++It)
    {
        AGoalPost* GoalPost = *It;
        if (IsValid(GoalPost) == true)
        {
            CachedGoalPosts.Add(GoalPost);
        }
    }

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] CachePlacedGoalPosts completed. GoalPostCount=%d"),
        CachedGoalPosts.Num()
    );
}

void APDGameModeBase::SpawnAndCacheBallCore()
{
    if (IsValid(CachedBallCore) == true)
    {
        UE_LOG(LogProjectD, Log, TEXT("[GameMode] SpawnAndCacheBallCore skipped. BallCore already exists."));
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] SpawnAndCacheBallCore failed. World is nullptr."));
        return;
    }

    if (BallCoreClass == nullptr)
    {
        UE_LOG(LogProjectD, Error, TEXT("[GameMode] SpawnAndCacheBallCore failed. BallCoreClass is nullptr."));
        return;
    }

    const FVector BallSpawnLocation = CalculateBallSpawnLocationFromGoals();
    const FRotator BallSpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ABallCore* SpawnedBallCore = World->SpawnActor<ABallCore>(
        BallCoreClass,
        BallSpawnLocation,
        BallSpawnRotation,
        SpawnParams
    );

    if (IsValid(SpawnedBallCore) == false)
    {
        UE_LOG(LogProjectD, Error, TEXT("[GameMode] SpawnAndCacheBallCore failed. SpawnedBallCore is invalid."));
        return;
    }

    CachedBallCore = SpawnedBallCore;
    CurrentRoundBallSpawnLocation = BallSpawnLocation;

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] SpawnAndCacheBallCore completed. SpawnLocation=(%.2f, %.2f, %.2f)"),
        BallSpawnLocation.X,
        BallSpawnLocation.Y,
        BallSpawnLocation.Z
    );
}

void APDGameModeBase::ResetRoundState()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] ResetRoundState failed. GameState is invalid."));
        return;
    }

    GS->CurrentBallHolder = nullptr;
    GS->GoalInstigator = nullptr;

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] Round state reset. CurrentRoundIndex=%d"),
        CurrentRoundIndex
    );
}

void APDGameModeBase::ResetPlacedGoalPostsForRound()
{
    for (AGoalPost* GoalPost : CachedGoalPosts)
    {
        if (IsValid(GoalPost) == false)
        {
            UE_LOG(LogProjectD, Warning, TEXT("[GameMode] ResetPlacedGoalPostsForRound skipped invalid GoalPost."));
            continue;
        }

        GoalPost->ResetGoalPost();
    }
}

void APDGameModeBase::ResetBallForRound()
{
    if (IsValid(CachedBallCore) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] ResetBallForRound failed. CachedBallCore is invalid."));
        return;
    }

    const FVector BallSpawnLocation = CalculateBallSpawnLocationFromGoals();
    if (BallSpawnLocation == FVector::ZeroVector)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] ResetBallForRound failed. BallSpawnLocation is zero vector."));
        return;
    }

    CurrentRoundBallSpawnLocation = BallSpawnLocation;

    CachedBallCore->ResetBallForRound(CurrentRoundBallSpawnLocation);

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] Ball reset for round. SpawnLocation=(%.2f, %.2f, %.2f)"),
        CurrentRoundBallSpawnLocation.X,
        CurrentRoundBallSpawnLocation.Y,
        CurrentRoundBallSpawnLocation.Z
    );
}

FVector APDGameModeBase::CalculateBallSpawnLocationFromGoals() const
{
    if (CachedGoalPosts.Num() < 3)
    {
        UE_LOG(
            LogProjectD,
            Warning,
            TEXT("[GameMode] CalculateBallSpawnLocationFromGoals failed. Need at least 3 GoalPosts. Count=%d"),
            CachedGoalPosts.Num()
        );

        return FVector::ZeroVector;
    }

    // Select 3 Idx
    int32 IndexA = FMath::RandRange(0, CachedGoalPosts.Num() - 1);

    int32 IndexB = INDEX_NONE;

    do
    {
        IndexB = FMath::RandRange(0, CachedGoalPosts.Num() - 1);
    } while (IndexB == IndexA);

    int32 IndexC = INDEX_NONE;

    do
    {
        IndexC = FMath::RandRange(0, CachedGoalPosts.Num() - 1);
    } while (IndexC == IndexA || IndexC == IndexB);

    AGoalPost* GoalPostA = CachedGoalPosts[IndexA];
    AGoalPost* GoalPostB = CachedGoalPosts[IndexB];
    AGoalPost* GoalPostC = CachedGoalPosts[IndexC];

    if (IsValid(GoalPostA) == false || IsValid(GoalPostB) == false || IsValid(GoalPostC) == false)
    {
        UE_LOG(
            LogProjectD,
            Warning,
            TEXT("[GameMode] CalculateBallSpawnLocationFromGoals failed. Selected GoalPost is invalid. IndexA=%d IndexB=%d IndexC=%d"),
            IndexA,
            IndexB,
            IndexC
        );

        return FVector::ZeroVector;
    }

    const FVector GoalLocationA = GoalPostA->GetActorLocation();
    const FVector GoalLocationB = GoalPostB->GetActorLocation();
    const FVector GoalLocationC = GoalPostC->GetActorLocation();

    const FVector CenterLocation = (GoalLocationA + GoalLocationB + GoalLocationC) / 3.0f;

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] CalculateBallSpawnLocationFromGoals selected random indices. A=%d B=%d C=%d Center=(%.2f, %.2f, %.2f)"),
        IndexA,
        IndexB,
        IndexC,
        CenterLocation.X,
        CenterLocation.Y,
        CenterLocation.Z
    );

    return CenterLocation;
}

FVector APDGameModeBase::BuildRespawnLocationForController(AController* Controller) const
{
    if (IsValid(Controller) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] BuildRespawnLocationForController failed. Controller is invalid."));
        return FVector::ZeroVector;
    }

    APDPlayerState* PS = Controller->GetPlayerState<APDPlayerState>();
    if (IsValid(PS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] BuildRespawnLocationForController failed. PlayerState is invalid."));
        return FVector::ZeroVector;
    }

    const int32 TeamId = static_cast<int32>(PS->GetTeamID());
    return BuildRespawnLocationFromTeam(TeamId);
}

FVector APDGameModeBase::BuildRespawnLocationFromTeam(int32 TeamId) const
{
    if (CurrentRoundBallSpawnLocation == FVector::ZeroVector)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] BuildRespawnLocationFromTeam failed. CurrentRoundBallSpawnLocation is zero vector."));
        return FVector::ZeroVector;
    }

    float BaseAngleDeg = 0.0f;

    if (TeamId == 0)
    {
        BaseAngleDeg = 90.0f;
    }
    else if (TeamId == 1)
    {
        BaseAngleDeg = 210.0f;
    }
    else if (TeamId == 2)
    {
        BaseAngleDeg = 330.0f;
    }
    else
    {
        BaseAngleDeg = 90.0f;
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] BuildRespawnLocationFromTeam used fallback angle. Invalid TeamId=%d"), TeamId);
    }

    const float BaseAngleRad = FMath::DegreesToRadians(BaseAngleDeg);
    const FVector Direction = FVector(FMath::Cos(BaseAngleRad), FMath::Sin(BaseAngleRad), 0.0f);

    return CurrentRoundBallSpawnLocation + (Direction * TeamRespawnRadiusFromBall) + FVector(0.0f, 0.0f, RespawnHeightOffset);
}

bool APDGameModeBase::TryFinishGameByScoreCondition()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TryFinishGameByScoreCondition failed. GameState is invalid."));
        return false;
    }

    if (TargetScoreToWin <= 0)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TryFinishGameByScoreCondition skipped. TargetScoreToWin must be greater than 0."));
        return false;
    }

    for (int32 TeamId = 0; TeamId < GS->TeamScores.Num(); ++TeamId)
    {
        const int32 TeamScore = GS->TeamScores[TeamId];

        if (TeamScore >= TargetScoreToWin)
        {
            UE_LOG(
                LogProjectD,
                Log,
                TEXT("[GameMode] Score win condition reached. TeamId=%d Score=%d TargetScoreToWin=%d"),
                TeamId,
                TeamScore,
                TargetScoreToWin
            );

            FinishGame(TeamId);
            return true;
        }
    }

    return false;
}

void APDGameModeBase::TriggerDroneSpawnOnBallPickup(APDPlayerState* HolderPlayerState)
{
    if (IsValid(HolderPlayerState) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TriggerDroneSpawnOnBallPickup failed. HolderPlayerState is invalid."));
        return;
    }

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] TriggerDroneSpawnOnBallPickup called. TeamId=%d"),
        static_cast<int32>(HolderPlayerState->GetTeamID())
    );

    // TODO - Drone Spawner
}

void APDGameModeBase::TriggerDroneExplosionOnGoal()
{
    UE_LOG(LogProjectD, Log, TEXT("[GameMode] TriggerDroneExplosionOnGoal called."));

    // TODO - Drone Spawner
}
