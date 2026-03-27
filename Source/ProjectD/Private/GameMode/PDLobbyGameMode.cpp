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
    int32 IncomingSize = UGameplayStatics::GetIntOption(Options, TEXT("TeamSize"), 1);
    FString LeaderSteamID = UGameplayStatics::ParseOption(Options, TEXT("LeaderSteamId"));

    bool bCanFit = false;

    UE_LOG(LogTemp, Warning, TEXT("PreLogin"));
    for (int32 i = 0; i < TEAM_COUNT; i++)
    {
        if (TeamInfos[i].LeaderSteamId == LeaderSteamID)
        {
            LoginInfo.Add(UniqueId.ToString(), LeaderSteamID);
            Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
            return;
        }
    }
    
    for (int32 i = 0; i < TEAM_COUNT; i++)
    {
        if (TeamInfos[i].PlayerCount + TeamInfos[i].PendingCount + IncomingSize <= MaxTeamSize)
        {
            if (!TeamInfos[i].LeaderSteamId.IsEmpty())
            {
                continue;
            }
            TeamInfos[i].LeaderSteamId = LeaderSteamID;
			TeamInfos[i].PendingCount = IncomingSize;
            LoginInfo.Add(UniqueId.ToString(), TeamInfos[i].LeaderSteamId);
            bCanFit = true;
            break;
        }
    }

    if (!bCanFit)
    {
        ErrorMessage = TEXT("No_Available_Team_Slot");
        return;
    }

    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void APDLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UE_LOG(LogTemp, Warning, TEXT("PostLogin"));
    APDPlayerState* PlayerState = NewPlayer->GetPlayerState<APDPlayerState>();
    if (!PlayerState)
    {
        return;
    }

    const FString NetIdKey = PlayerState->GetUniqueId().ToString(); 
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
        if (--TeamInfos[SelectedTeam].PendingCount == 0)
        {
			TeamInfos[SelectedTeam].LeaderSteamId = TEXT("");
        }
		PlayerState->SetTeamID(TeamInfos[SelectedTeam].TeamID);
		LoginInfo.Remove(NetIdKey);
        UE_LOG(LogTemp, Warning, TEXT("플레이어 %s가 %d번 팀에 배정됨. 현재 팀 인원: %d"), *PlayerState->GetPlayerName(), SelectedTeam, TeamInfos[SelectedTeam].PlayerCount);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("플레이어 %s가 팀에 배정되지 못함"), *PlayerState->GetPlayerName());
    }

    if (auto* PC = Cast<APDLobbyPlayerController>(NewPlayer))
    {
        PC->Client_RequestCharacterCustomInfo();
        PC->Client_RequestDisplayName();
    }
    BroadcastLobbyTeamInfos();
    UpdateSessionMetadata();
	TryGameStart(false);
}

void APDLobbyGameMode::Logout(AController* Exiting)
{
    APDPlayerState* PlayerState = Exiting ? Exiting->GetPlayerState<APDPlayerState>() : nullptr;
    if (PlayerState && static_cast<int>(PlayerState->GetTeamID()) < TEAM_COUNT)
    {
		TeamInfos[static_cast<int>(PlayerState->GetTeamID())].PlayerCount--;
    }

    Super::Logout(Exiting);

    BroadcastLobbyTeamInfos();
    UpdateSessionMetadata();
}

void APDLobbyGameMode::CreateDedicatedSession()
{
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
    }

    //const FString TravelURL = TEXT("/Game/MiddleEasternTown/Levels/L_MiddleEasternTown");
    const FString TravelURL = TEXT("/Game/LakeTown/Maps/Demonstration");
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
        Snapshot.bHasTeamMember_0 = false;
        Snapshot.bHasTeamMember_1 = false;
        Snapshot.TeamMemberDisplayName_0.Reset();
        Snapshot.TeamMemberDisplayName_1.Reset();
        Snapshot.TeamMemberId_0 = FBPUniqueNetId();
        Snapshot.TeamMemberId_1 = FBPUniqueNetId();
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

            const int32 TeamIndex = static_cast<int32>(PDPlayerState->GetTeamID());
            if (!TeamInfoSnapshot.IsValidIndex(TeamIndex))
            {
                continue;
            }

            FTeamInfo& Snapshot = TeamInfoSnapshot[TeamIndex];
            if (!Snapshot.bHasTeamMember_0)
            {
                Snapshot.bHasTeamMember_0 = true;
                Snapshot.TeamMemberDisplayName_0 = PDPlayerState->GetDisplayName();
                Snapshot.TeamMemberId_0.SetUniqueNetId(PDPlayerState->GetUniqueId().GetUniqueNetId());
            }
            else if (!Snapshot.bHasTeamMember_1)
            {
                Snapshot.bHasTeamMember_1 = true;
                Snapshot.TeamMemberDisplayName_1 = PDPlayerState->GetDisplayName();
                Snapshot.TeamMemberId_1.SetUniqueNetId(PDPlayerState->GetUniqueId().GetUniqueNetId());
            }
        }
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
