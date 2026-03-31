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
#include "Object/BallSpawnPosition.h"
#include "ProjectD/ProjectD.h"
#include "AI/MassAI/DroneSpawner.h"
#include "AI/MassAI/CheckDroneAllExplodeSubsystem.h"

APDGameModeBase::APDGameModeBase()
{
    TotalGameDurationSec = 1800;

    TargetScoreToWin = 1000;

    NextRoundDelaySec = 3.0f;
    InitialPreRoundDurationSec = 10.0f;

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

    CacheRoundActors();
    CacheDroneSpawner();

    bWorldReady = true;

    UE_LOG(LogProjectD, Log, TEXT("[GameMode] World ready."));
}

void APDGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    const FString ExpectedPlayersOption = UGameplayStatics::ParseOption(Options, TEXT("ExpectedPlayers"));
    ExpectedTravelPlayerCount = FCString::Atoi(*ExpectedPlayersOption);

    CurrentRoundIndex = 1;
    RoundPhase = ERoundPhase::Waiting;

    bInitialPreRoundStarted = false;
    bInitialPreRoundFinished = false;
    bGameTimerStarted = false;

    bMatchFlowInitialized = true;
    bWorldReady = false;

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] InitGame. Map=%s ExpectedTravelPlayerCount=%d Options=%s"),
        *MapName,
        ExpectedTravelPlayerCount,
        *Options
    );
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

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this]()
        {
            for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                if (APDPlayerController* PC = Cast<APDPlayerController>(It->Get()))
                {
                    PC->ClientTravel(TEXT("/Game/Lobby/Lobby10"), ETravelType::TRAVEL_Absolute);
                }
            }

            FTimerHandle TimerHandle;

            GetWorldTimerManager().SetTimer(TimerHandle, [this]()
                {
                    const FString LobbyMapPath = TEXT("/Game/ProjectD/Maps/ServerLobbyLevel?listen");
                    GetWorld()->ServerTravel(LobbyMapPath);
                }, 2.0f, false);
        }, 20.0f, false); 
}

void APDGameModeBase::HandleBallPickedUp(APDPlayerState* HolderPlayerState, ABallCore* Ball)
{
    if (RoundPhase != ERoundPhase::InRound)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleBallPickedUp skipped. RoundPhase is not InRound."));
        return;
    }

    if (bGoalProcessingThisRound == true)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleBallPickedUp skipped. Goal is already processing."));
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

    if (bDroneSpawnTriggeredThisRound == true)
    {
        UE_LOG(LogProjectD, Log, TEXT("[GameMode] HandleBallPickedUp skipped. Drone already spawned this round."));
        return;
    }

    bDroneSpawnTriggeredThisRound = true;
    TriggerDroneSpawnOnBallPickup(HolderPlayerState);
}

void APDGameModeBase::HandleGoalEntered(AGoalPost* GoalPost, ABallCore* Ball)
{
    if (RoundPhase != ERoundPhase::InRound)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGoalEntered skipped. RoundPhase is not InRound."));
        return;
    }

    if (bGoalProcessingThisRound == true)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGoalEntered skipped. Goal already processing."));
        return;
    }

    if (IsValid(GoalPost) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGoalEntered failed. GoalPost is invalid."));
        return;
    }

    if (IsValid(Ball) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleGoalEntered failed. Ball is invalid."));
        return;
    }

    bGoalProcessingThisRound = true;

    TriggerDroneExplosionOnGoal();
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

void APDGameModeBase::CheckAllPlayersReady()
{
    if (bInitialPreRoundStarted || bInitialPreRoundFinished)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AGameStateBase* GS = World->GetGameState();
    if (!GS)
    {
        return;
    }

    if (GS->PlayerArray.Num() < ExpectedTravelPlayerCount)
    {
        return;
    }

    int32 ReadyCount = 0;
    for (APlayerState* PS : GS->PlayerArray)
    {
        if (APDPlayerState* PDPS = Cast<APDPlayerState>(PS))
        {
            if (PDPS->bClientReady)
            {
                ReadyCount++;
            }
        }
    }

    UE_LOG(LogProjectD, Log, TEXT("[GameMode] Checking Ready Status: %d / %d"), ReadyCount, ExpectedTravelPlayerCount);

    if (ReadyCount >= ExpectedTravelPlayerCount)
    {
        TryStartInitialPreRound();
    }
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

	if (APDPlayerState* PS = Controller->GetPlayerState<APDPlayerState>())
	{
		PS->SetReviveState();
	}
    else
	{
		UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PlayerRespawn. PlayerState is invalid."));
		return;
	}

	const FRotator RespawnRotation = (CurrentRoundBallSpawnLocation - RespawnLocation).Rotation();
	Pawn->TeleportTo(RespawnLocation, RespawnRotation);
}

void APDGameModeBase::StartInitialPreRound()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartInitialPreRound failed. GameState is invalid."));
        return;
    }

    if (RoundPhase == ERoundPhase::GameEnded)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartInitialPreRound skipped. RoundPhase is GameEnded."));
        return;
    }

    if (bInitialPreRoundFinished == true)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartInitialPreRound skipped. Initial pre-round already finished."));
        return;
    }

    if (RoundPhase != ERoundPhase::Waiting)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartInitialPreRound skipped. RoundPhase is not Waiting."));
        return;
    }

    ResetRoundState();
    ResetPlacedGoalPostsForRound();
    ResetBallForRound();

    SetAllPlayersMovementLocked(true);
    SetAllPlayersShopEnabled(true);

    RoundPhase = ERoundPhase::InitPreRound;

    GetWorldTimerManager().ClearTimer(InitialPreRoundTimerHandle);
    GetWorldTimerManager().SetTimer(
        InitialPreRoundTimerHandle,
        this,
        &APDGameModeBase::FinishInitialPreRound,
        InitialPreRoundDurationSec,
        false
    );

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] InitialPreRound started. Duration=%.2f"),
        InitialPreRoundDurationSec
    );

}

void APDGameModeBase::FinishInitialPreRound()
{
    if (RoundPhase != ERoundPhase::InitPreRound)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] FinishInitialPreRound skipped. RoundPhase is not InitialPreRound."));
        return;
    }

    SetAllPlayersShopEnabled(false);
    SetAllPlayersMovementLocked(false);

    bInitialPreRoundFinished = true;

    TeleportAllPlayersToRoundStart();

    StartGameTimer();
    StartFirstRound();

    UE_LOG(LogProjectD, Log, TEXT("[GameMode] InitialPreRound finished. First round will start."));
}

void APDGameModeBase::StartFirstRound()
{
    if (RoundPhase == ERoundPhase::GameEnded)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartFirstRound skipped. RoundPhase is GameEnded."));
        return;
    }

    RoundPhase = ERoundPhase::InRound;

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] First round started. CurrentRoundIndex=%d"),
        CurrentRoundIndex
    );
}

void APDGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (APDPlayerController* PC = Cast<APDPlayerController>(NewPlayer))
    {
        PC->Client_SetExpectedPlayerCount(ExpectedTravelPlayerCount);
    }

    UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PostLogin Try"));
    if (IsValid(NewPlayer) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] PostLogin failed. NewPlayer is invalid."));
        return;
    }

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] PostLogin called. Controller=%s PlayerState=%s"),
        *GetNameSafe(NewPlayer),
        *GetNameSafe(NewPlayer->PlayerState)
    );

    BindPlayerDelegates(NewPlayer);
    RegisterTravelReadyPlayer(NewPlayer);
}

void APDGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);

    if (APDPlayerController* PC = Cast<APDPlayerController>(NewPlayer))
    {
        PC->Client_SetExpectedPlayerCount(ExpectedTravelPlayerCount);

        UE_LOG(LogProjectD, Log, TEXT("[GameMode] Seamless Travel Player Started: %s"), *PC->GetName());
    }

    if (IsValid(NewPlayer) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] HandleStartingNewPlayer failed. NewPlayer is invalid."));
        return;
    }

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] HandleStartingNewPlayer called. Controller=%s PlayerState=%s"),
        *GetNameSafe(NewPlayer),
        *GetNameSafe(NewPlayer->PlayerState)
    );

    BindPlayerDelegates(NewPlayer);
    RegisterTravelReadyPlayer(NewPlayer);
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

void APDGameModeBase::StartGameTimer()
{
    if (bGameTimerStarted == true)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartGameTimer skipped. Timer already started."));
        return;
    }

    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (IsValid(GS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] StartGameTimer failed. GameState is invalid."));
        return;
    }

    GS->RemainingTimeSec = TotalGameDurationSec;

    GetWorldTimerManager().ClearTimer(GameTimerHandle);
    GetWorldTimerManager().SetTimer(
        GameTimerHandle,
        this,
        &APDGameModeBase::OnGameTick,
        1.0f,
        true
    );

    bGameTimerStarted = true;

    UE_LOG(LogProjectD, Log, TEXT("[GameMode] Game timer started."));
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
    CachePlacedBallSpawnPositions();
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

void APDGameModeBase::CachePlacedBallSpawnPositions()
{
    CachedBallSpawnPositions.Empty();

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] CachePlacedBallSpawnPositions failed. World is nullptr."));
        return;
    }

    for (TActorIterator<ABallSpawnPosition> It(World); It; ++It)
    {
        ABallSpawnPosition* BallSpawnPosition = *It;
        if (IsValid(BallSpawnPosition) == true)
        {
            CachedBallSpawnPositions.Add(BallSpawnPosition);
        }
    }

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] CachePlacedBallSpawnPositions completed. BallSpawnPositionCount=%d"),
        CachedBallSpawnPositions.Num()
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

    const FVector BallSpawnLocation = GetRandomBallSpawnLocation();
    if (BallSpawnLocation == FVector::ZeroVector)
    {
        UE_LOG(LogProjectD, Error, TEXT("[GameMode] SpawnAndCacheBallCore failed. BallSpawnLocation is zero vector."));
        return;
    }

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

void APDGameModeBase::CacheDroneSpawner()
{
    CachedDroneSpawner = nullptr;

    UWorld* World = GetWorld();
    if (IsValid(World) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] CacheDroneSpawner failed. World is invalid."));
        return;
    }

    for (TActorIterator<ADroneSpawner> It(World); It; ++It)
    {
        ADroneSpawner* DroneSpawner = *It;
        if (IsValid(DroneSpawner) == true)
        {
            CachedDroneSpawner = DroneSpawner;
            break;
        }
    }

    if (IsValid(CachedDroneSpawner) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] CacheDroneSpawner failed. DroneSpawner not found."));
    }
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

    bDroneSpawnTriggeredThisRound = false;
    bGoalProcessingThisRound = false;

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

    const FVector BallSpawnLocation = GetRandomBallSpawnLocation();
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

FVector APDGameModeBase::GetRandomBallSpawnLocation() const
{
    if (CachedBallSpawnPositions.Num() <= 0)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] GetRandomBallSpawnLocation failed. CachedBallSpawnPositions is empty."));
        return FVector::ZeroVector;
    }

    const int32 RandomIndex = FMath::RandRange(0, CachedBallSpawnPositions.Num() - 1);
    ABallSpawnPosition* SelectedSpawnPosition = CachedBallSpawnPositions[RandomIndex];

    if (IsValid(SelectedSpawnPosition) == false)
    {
        UE_LOG(
            LogProjectD,
            Warning,
            TEXT("[GameMode] GetRandomBallSpawnLocation failed. SelectedSpawnPosition is invalid. Index=%d"),
            RandomIndex
        );
        return FVector::ZeroVector;
    }

    const FVector SpawnLocation = SelectedSpawnPosition->GetActorLocation();

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] GetRandomBallSpawnLocation selected. Index=%d Location=(%.2f, %.2f, %.2f)"),
        RandomIndex,
        SpawnLocation.X,
        SpawnLocation.Y,
        SpawnLocation.Z
    );

    return SpawnLocation;
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

    if (IsValid(CachedDroneSpawner) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TriggerDroneSpawnOnBallPickup failed. CachedDroneSpawner is invalid."));
        return;
    }

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] TriggerDroneSpawnOnBallPickup called. TeamId=%d"),
        static_cast<int32>(HolderPlayerState->GetTeamID())
    );

    CachedDroneSpawner->SpawnDrones();
}

void APDGameModeBase::TriggerDroneExplosionOnGoal()
{
    UWorld* World = GetWorld();
    if (IsValid(World) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TriggerDroneExplosionOnGoal failed. World is invalid."));
        return;
    }

    UCheckDroneAllExplodeSubsystem* ExplodeSubsystem = World->GetSubsystem<UCheckDroneAllExplodeSubsystem>();
    if (IsValid(ExplodeSubsystem) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TriggerDroneExplosionOnGoal failed. ExplodeSubsystem is invalid."));
        return;
    }

    UE_LOG(LogProjectD, Log, TEXT("[GameMode] TriggerDroneExplosionOnGoal called."));

    ExplodeSubsystem->RequestExplodeAllDrones();
}

void APDGameModeBase::TeleportAllPlayersToRoundStart()
{
    UWorld* World = GetWorld();
    if (IsValid(World) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TeleportAllPlayersToRoundStart failed. World is invalid."));
        return;
    }

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (IsValid(PC) == false)
        {
            continue;
        }

        TeleportPlayerToRoundStart(PC);
    }
}

void APDGameModeBase::TeleportPlayerToRoundStart(AController* Controller)
{
    if (IsValid(Controller) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TeleportPlayerToRoundStart failed. Controller is invalid."));
        return;
    }

    APawn* Pawn = Controller->GetPawn();
    if (IsValid(Pawn) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TeleportPlayerToRoundStart failed. Pawn is invalid."));
        return;
    }

    const FVector SpawnLocation = BuildRespawnLocationForController(Controller);
    if (SpawnLocation == FVector::ZeroVector)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TeleportPlayerToRoundStart failed. SpawnLocation is zero vector."));
        return;
    }

    const FRotator SpawnRotation = (CurrentRoundBallSpawnLocation - SpawnLocation).Rotation();
    Pawn->TeleportTo(SpawnLocation, SpawnRotation);
}

void APDGameModeBase::SetAllPlayersMovementLocked(bool bLocked)
{
    UWorld* World = GetWorld();
    if (IsValid(World) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] SetAllPlayersMovementLocked failed. World is invalid."));
        return;
    }

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (IsValid(PC) == false)
        {
            continue;
        }

        SetPlayerMovementLocked(PC, bLocked);
    }
}

void APDGameModeBase::SetPlayerMovementLocked(AController* Controller, bool bLocked)
{
    if (IsValid(Controller) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] SetPlayerMovementLocked failed. Controller is invalid."));
        return;
    }

    UE_LOG(LogProjectD, Warning, TEXT("[GameMode] SetPlayerMovementLocked Success."));
}

void APDGameModeBase::SetAllPlayersShopEnabled(bool bEnabled)
{
    UWorld* World = GetWorld();
    if (IsValid(World) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] SetAllPlayersShopEnabled failed. World is invalid."));
        return;
    }

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (IsValid(PC) == false)
        {
            continue;
        }

        SetPlayerShopEnabled(PC, bEnabled);
    }
}

void APDGameModeBase::SetPlayerShopEnabled(AController* Controller, bool bEnabled)
{
    if (IsValid(Controller) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] SetPlayerShopEnabled failed. Controller is invalid."));
        return;
    }

    // TODO : Shop Block & Can Open? (By bEnabled)
    UE_LOG(LogProjectD, Warning, TEXT("[GameMode] SetPlayerShopEnabled Success."));
}

void APDGameModeBase::BindPlayerDelegates(APlayerController* NewPlayer)
{
    if (IsValid(NewPlayer) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] BindPlayerDelegates failed. NewPlayer is invalid."));
        return;
    }

    APDPlayerState* PS = NewPlayer->GetPlayerState<APDPlayerState>();
    if (IsValid(PS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] BindPlayerDelegates failed. PlayerState is invalid."));
        return;
    }

    UPDAttributeSetBase* AS = PS->GetPDAttributeSetBase();
    if (IsValid(AS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] BindPlayerDelegates failed. AttributeSet is invalid."));
        return;
    }

    AS->OnOutOfHealth.AddUniqueDynamic(this, &APDGameModeBase::OnPlayerOutOfHealth);

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] BindPlayerDelegates completed. PlayerState=%s"),
        *GetNameSafe(PS)
    );
}

void APDGameModeBase::RegisterTravelReadyPlayer(APlayerController* NewPlayer)
{
    if (IsValid(NewPlayer) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] RegisterTravelReadyPlayer failed. NewPlayer is invalid."));
        return;
    }

    APlayerState* PS = NewPlayer->PlayerState;
    if (IsValid(PS) == false)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] RegisterTravelReadyPlayer failed. PlayerState is invalid."));
        return;
    }

    TravelReadyPlayerStates.Add(PS);

    UE_LOG(
        LogProjectD,
        Log,
        TEXT("[GameMode] RegisterTravelReadyPlayer. ReadyCount=%d ExpectedTravelPlayerCount=%d PlayerState=%s"),
        TravelReadyPlayerStates.Num(),
        ExpectedTravelPlayerCount,
        *GetNameSafe(PS)
    );
}

void APDGameModeBase::TryStartInitialPreRound()
{
    if (bInitialPreRoundStarted == true)
    {
        return;
    }

    if (bInitialPreRoundFinished == true)
    {
        return;
    }

    if (bMatchFlowInitialized == false)
    {
        UE_LOG(LogProjectD, Log, TEXT("[GameMode] TryStartInitialPreRound waiting. MatchFlow is not initialized."));
        return;
    }

    if (bWorldReady == false)
    {
        UE_LOG(LogProjectD, Log, TEXT("[GameMode] TryStartInitialPreRound waiting. World is not ready."));
        return;
    }

    if (RoundPhase != ERoundPhase::Waiting)
    {
        return;
    }

    if (ExpectedTravelPlayerCount <= 0)
    {
        UE_LOG(LogProjectD, Warning, TEXT("[GameMode] TryStartInitialPreRound skipped. ExpectedTravelPlayerCount must be greater than 0."));
        return;
    }

    if (IsValid(CachedBallCore) == false)
    {
        UE_LOG(LogProjectD, Log, TEXT("[GameMode] TryStartInitialPreRound waiting. CachedBallCore is invalid."));
        return;
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APDPlayerController* PC = Cast<APDPlayerController>(It->Get()))
        {
            PC->Client_OnGameStarted();
        }
    }

    bInitialPreRoundStarted = true;
    StartInitialPreRound();
}

void APDGameModeBase::OnReadyTimeout()
{
    if (bInitialPreRoundStarted)
    {
        return;
    }

    UE_LOG(LogProjectD, Warning, TEXT("[GameMode] Ready Timeout! Starting game forcibly."));

    TryStartInitialPreRound();
}
