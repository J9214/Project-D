// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/PDLobbyGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h" 
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/PDPlayerState.h"
#include "Controller/PDServerLobbyPlayerController.h"
#include "Engine/NetDriver.h"
#include "GameFramework/GameStateBase.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include <Controller/PDLobbyPlayerController.h>

APDLobbyGameMode::APDLobbyGameMode()
{
    for (int32 i = 0; i < TEAM_COUNT; i++)
    {
        TeamInfos[i].TeamID = static_cast<ETeamType>(i);
        TeamInfos[i].LeaderSteamId = TEXT("");
		TeamInfos[i].PlayerCount = 0;
		TeamInfos[i].PendingCount = 0;
        TeamInfos[i].MaxPlayerCount = MaxTeamSize;
    }

    PendingIncomingPlayers = 0;
    bUseSeamlessTravel = true;
}

void APDLobbyGameMode::TravelToLobby10()
{
    if (!GetWorld() || GetWorld()->IsInSeamlessTravel())
    {
        UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] TravelToLobby10 ignored. World invalid or already in seamless travel."));
        return;
    }

    const FString TravelURL = TEXT("/Game/ProjectD/Maps/level_Temp");
    UE_LOG(LogTemp, Log, TEXT("[LobbyGameMode] ServerTravel to %s"), *TravelURL);
    GetWorld()->ServerTravel(TravelURL);
}

void APDLobbyGameMode::BeginPlay()
{
    Super::BeginPlay();
    LobbyMatchStartServerTimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    if (IsRunningDedicatedServer() && !bIsSessionCreating)
    {
        bIsSessionCreating = true;
        CreateDedicatedSession();
    }
}

void APDLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{

#if UE_EDITOR
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
#else
    int32 IncomingSize = UGameplayStatics::GetIntOption(Options, TEXT("TeamSize"), 1);
    FString LeaderSteamID = UGameplayStatics::ParseOption(Options, TEXT("LeaderSteamId"));

    {
        FScopeLock Lock(&TeamLock);

        bool bCanFit = false;

        if (LoginInfo.Contains(UniqueId.ToString()))
        {
            Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
            return;
        }

        int32 SelectedTeamIdx = -1;

        for (int32 i = 0; i < TEAM_COUNT; i++)
        {
            if (TeamInfos[i].LeaderSteamId == LeaderSteamID && !LeaderSteamID.IsEmpty())
            {
                if (TeamInfos[i].PlayerCount + TeamInfos[i].PendingCount + IncomingSize <= MaxTeamSize)
                {
                    SelectedTeamIdx = i;
                    break;
                }
            }
        }

        if (SelectedTeamIdx == -1)
        {
            for (int32 i = 0; i < TEAM_COUNT; i++)
            {
                if (TeamInfos[i].LeaderSteamId.IsEmpty() && (TeamInfos[i].PlayerCount + TeamInfos[i].PendingCount == 0))
                {
                    if (IncomingSize <= MaxTeamSize)
                    {
                        SelectedTeamIdx = i;
                        TeamInfos[i].LeaderSteamId = LeaderSteamID;
                        break;
                    }
                }
            }
        }

        if (SelectedTeamIdx != -1)
        {
            TeamInfos[SelectedTeamIdx].PendingCount += IncomingSize;
            LoginInfo.Add(UniqueId.ToString(), LeaderSteamID);

            UE_LOG(LogTemp, Log, TEXT("[PreLogin] Team %d Reserved (Leader: %s)"), SelectedTeamIdx, *LeaderSteamID);
        }
        else
        {
            ErrorMessage = TEXT("No_Available_Team_Slot");
            UE_LOG(LogTemp, Warning, TEXT("[PreLogin] Denied: Could not find or fit into a team for Leader %s"), *LeaderSteamID);
        }
    }
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

#endif
}

void APDLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

#if UE_EDITOR

    APDPlayerState* PlayerState = NewPlayer->GetPlayerState<APDPlayerState>();
    if (!PlayerState)
    {
        return;
    }
    int32 TestTeamIdx = 0;
    PlayerState->SetTeamID(static_cast<ETeamType>(TestTeamIdx));
    TeamInfos[TestTeamIdx].PlayerCount++;

    UE_LOG(LogTemp, Warning, TEXT("[Editor] Player %s assigned to Team %d"), *PlayerState->GetPlayerName(), TestTeamIdx);

    BroadcastLobbyTeamInfos();
    TryGameStart(true);
#else
    APDPlayerState* PlayerState = NewPlayer->GetPlayerState<APDPlayerState>();
    if (!PlayerState)
    {
        return;
    }

    const FString NetIdKey = PlayerState->GetUniqueId().ToString(); 
    ETeamType AssignedTeamID = ETeamType::None;

    {
        FScopeLock Lock(&TeamLock);
        FString* FoundLeaderSteamID = LoginInfo.Find(NetIdKey);

        if (!FoundLeaderSteamID)
        {
            UE_LOG(LogTemp, Warning, TEXT("LoginInfo missing: %s"), *NetIdKey);
            return;
        }

        int32 SelectedTeam = -1;
        for (int32 i = 0; i < TEAM_COUNT; i++)
        {
            if (TeamInfos[i].LeaderSteamId == *FoundLeaderSteamID)
            {
                SelectedTeam = i;
                break;
            }
        }

        if (SelectedTeam != -1)
        {
            TeamInfos[SelectedTeam].PlayerCount++;
            AssignedTeamID = TeamInfos[SelectedTeam].TeamID;

            if (--TeamInfos[SelectedTeam].PendingCount <= 0)
            {
                TeamInfos[SelectedTeam].PendingCount = 0;
                TeamInfos[SelectedTeam].LeaderSteamId = TEXT("");
            }

            LoginInfo.Remove(NetIdKey);
            UE_LOG(LogTemp, Warning, TEXT("플레이어 %s가 %d번 팀에 배정됨. 현재 팀 인원: %d"), *PlayerState->GetPlayerName(), SelectedTeam, TeamInfos[SelectedTeam].PlayerCount);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("플레이어 %s가 팀에 배정되지 못함"), *PlayerState->GetPlayerName());
            NewPlayer->ClientTravel(TEXT("/Game/ProjectD/Maps/Main_Menu"), ETravelType::TRAVEL_Absolute);
            LoginInfo.Remove(NetIdKey);
            return;
        }
    }

    PlayerState->SetTeamID(AssignedTeamID);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[LobbyDisplayName] PostLogin Sync Team=%d PlayerName=[%s] DisplayName=[%s] Resolved=[%s] NetId=[%s]"),
        AssignedTeamID,
        *PlayerState->GetPlayerName(),
        *PlayerState->GetDisplayName(),
        *PlayerState->GetResolvedDisplayName(),
        *PlayerState->GetUniqueId().ToString());

    BroadcastLobbyTeamInfos();
    UpdateSessionMetadata();
	TryGameStart(false);

#endif
}

void APDLobbyGameMode::Logout(AController* Exiting)
{
    {
        FScopeLock Lock(&TeamLock);

        APDPlayerState* PlayerState = Exiting ? Exiting->GetPlayerState<APDPlayerState>() : nullptr;
        if (PlayerState)
        {
            int32 TeamIndex = static_cast<int>(PlayerState->GetTeamID());
            if (TeamIndex >= 0 && TeamIndex < TEAM_COUNT)
            {
                TeamInfos[TeamIndex].PlayerCount = FMath::Max(0, TeamInfos[TeamIndex].PlayerCount - 1);
            }
        }

    }

    Super::Logout(Exiting);
    BroadcastLobbyTeamInfos();
    UpdateSessionMetadata();
}

void APDLobbyGameMode::CreateDedicatedSession()
{
#if !UE_EDITOR
    
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(FName("Steam"));
    if (OSS)
    {
        IOnlineSessionPtr SessionInterface = OSS->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
            if (ExistingSession)
            {
                SessionInterface->DestroySession(NAME_GameSession);
            }
            CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &APDLobbyGameMode::OnCreateSessionComplete);
            CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

            FOnlineSessionSettings Settings;
            Settings.bIsDedicated = true;
            Settings.bShouldAdvertise = true;
            Settings.bIsLANMatch = false;
            Settings.NumPublicConnections = 6;
            Settings.bAllowJoinInProgress = true;
            Settings.bUsesPresence = false;
            Settings.bAllowJoinViaPresence = false;

            int32 QueryPort = 27015;
            FParse::Value(FCommandLine::Get(), TEXT("QueryPort="), QueryPort);

            Settings.Set(FName(TEXT("GameFilter")), FString("UnrealSteamTestLobbyEEEUTTR"), EOnlineDataAdvertisementType::ViaOnlineService);
            Settings.Set(FName(TEXT("MAX_FIT")), 3, EOnlineDataAdvertisementType::ViaOnlineService);

            SessionInterface->CreateSession(0, NAME_GameSession, Settings);
        }
    }

#endif
}

void APDLobbyGameMode::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        LobbyMatchStartServerTimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        UE_LOG(LogTemp, Log, TEXT("Dedi Server: Session Created and Broadcasting to Steam!"));
    }
}

void APDLobbyGameMode::UpdateSessionMetadata()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(FName("Steam"));
    if (!OSS)
    {
        return;
    }

    IOnlineSessionPtr SessionInterface = OSS->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        return;
    }

    FOnlineSessionSettings* Settings = SessionInterface->GetSessionSettings(NAME_GameSession);
    if (!Settings)
    {
        return;
    }

    int32 MaxFit = 0;
    for (int32 i = 0; i < 3; i++)
    {
        int32 FreeSpace = MaxTeamSize - TeamInfos[i].PlayerCount;
        if (FreeSpace > MaxFit)
        {
            MaxFit = FreeSpace;
        }
    }

    Settings->Set(FName(TEXT("MAX_FIT")), MaxFit, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionInterface->UpdateSession(NAME_GameSession, *Settings);

    UE_LOG(LogTemp, Warning, TEXT("세션 메타데이터 갱신: MAX_FIT = %d"), MaxFit);
}

void APDLobbyGameMode::TryGameStart(bool bIsTest)
{
    UE_LOG(LogTemp, Warning, TEXT("TryGameStart"));
    if (GetWorld()->IsInSeamlessTravel())
    {
		return;
    }

    if (!bIsTest)
    {
        for(int i = 0 ; i < 3; i++)
        {
            if (TeamInfos[i].PlayerCount != MaxTeamSize)
            {
                return;
            }
		}
    }
    else
    {
#if !UE_EDITOR
        IOnlineSubsystem* OSS = IOnlineSubsystem::Get(FName("Steam"));
        if (!OSS)
        {
            return;
        }

        IOnlineSessionPtr SessionInterface = OSS->GetSessionInterface();
        if (!SessionInterface.IsValid())
        {
            return;
        }

        FOnlineSessionSettings* Settings = SessionInterface->GetSessionSettings(NAME_GameSession);
        if (!Settings)
        {
            return;
        }

        Settings->Set(FName(TEXT("MAX_FIT")), 0, EOnlineDataAdvertisementType::ViaOnlineService);
        SessionInterface->UpdateSession(NAME_GameSession, *Settings);
#endif

    }

    //const FString TravelURL = TEXT("/Game/MiddleEasternTown/Levels/L_MiddleEasternTown");
    const int32 PlayerNums = GetNumPlayers();
    const FString TravelURL = FString::Printf(
        TEXT("/Game/LakeTown/Maps/Demonstration?ExpectedPlayers=%d"),
        PlayerNums
    );
    GetWorld()->ServerTravel(TravelURL);
}

void APDLobbyGameMode::BroadcastLobbyTeamInfos()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<FTeamInfo> TeamInfoSnapshot;
    TeamInfoSnapshot.Reserve(TEAM_COUNT);

    for (int32 TeamIndex = 0; TeamIndex < TEAM_COUNT; ++TeamIndex)
    {
        FTeamInfo Snapshot = TeamInfos[TeamIndex];
        Snapshot.MaxPlayerCount = MaxTeamSize;
        TeamInfoSnapshot.Add(MoveTemp(Snapshot));
    }

    AGameStateBase* CurrentGameState = GameState;
    if (CurrentGameState)
    {
        for (APlayerState* BasePlayerState : CurrentGameState->PlayerArray)
        {
            APDPlayerState* PDPlayerState = Cast<APDPlayerState>(BasePlayerState);
            if (!PDPlayerState)
            {
                continue;
            }

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[LobbyTeamPlayerState] Team=%d PlayerName=[%s] DisplayName=[%s] ResolvedDisplayName=[%s] NetId=[%s]"),
                static_cast<int32>(PDPlayerState->GetTeamID()),
                *PDPlayerState->GetPlayerName(),
                *PDPlayerState->GetDisplayName(),
                *PDPlayerState->GetResolvedDisplayName(),
                *PDPlayerState->GetUniqueId().ToString());
        }
    }

    for (int32 TeamIndex = 0; TeamIndex < TeamInfoSnapshot.Num(); ++TeamIndex)
    {
        const FTeamInfo& Snapshot = TeamInfoSnapshot[TeamIndex];
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[LobbyTeamSnapshot] Team=%d Count=%d Pending=%d Max=%d Leader=[%s] MatchStart=%.2f"),
            TeamIndex,
            Snapshot.PlayerCount,
            Snapshot.PendingCount,
            Snapshot.MaxPlayerCount,
            *Snapshot.LeaderSteamId,
            LobbyMatchStartServerTimeSec);
    }

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APDServerLobbyPlayerController* ServerLobbyPC = Cast<APDServerLobbyPlayerController>(It->Get());
        if (!ServerLobbyPC)
        {
            continue;
        }

        ServerLobbyPC->Client_UpdateLobbyTeamInfos(TeamInfoSnapshot, LobbyMatchStartServerTimeSec);
    }
}
