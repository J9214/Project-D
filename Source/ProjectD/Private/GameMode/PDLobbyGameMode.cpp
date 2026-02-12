// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/PDLobbyGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h" 
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/PDPlayerState.h"
#include "Engine/NetDriver.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

APDLobbyGameMode::APDLobbyGameMode()
{
    for (int32 i = 0; i < 3; i++)
    {
        TeamCounts[i] = 0;
    }
    PendingIncomingPlayers = 0;
}

void APDLobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (IsRunningDedicatedServer() && !bIsSessionCreating)
    {
        bIsSessionCreating = true;
        CreateDedicatedSession();
    }
}

void APDLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    int32 IncomingSize = UGameplayStatics::GetIntOption(Options, TEXT("TeamSize"), 1);

    bool bCanFit = false;

    for (int32 i = 0; i < 3; i++)
    {
        if (TeamCounts[i] + IncomingSize <= MaxTeamSize)
        {
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

    APDPlayerState* PlayerState = NewPlayer->GetPlayerState<APDPlayerState>();
    if (!PlayerState)
    {
        return;
    }

    int32 SelectedTeam = -1;
    for (int32 i = 0; i < 3; i++)
    {
        if (TeamCounts[i] < MaxTeamSize)
        {
            SelectedTeam = i;
            break;
        }
    }

    if (SelectedTeam != -1)
    {
        PlayerState->TeamId = SelectedTeam;
        TeamCounts[SelectedTeam]++;

        UE_LOG(LogTemp, Warning, TEXT("플레이어 %s가 %d번 팀에 배정됨. 현재 팀 인원: %d"), *PlayerState->GetPlayerName(), SelectedTeam, TeamCounts[SelectedTeam]);
    }

    UpdateSessionMetadata();
}

void APDLobbyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

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
            Settings.NumPublicConnections = 10;
            Settings.bAllowJoinInProgress = true;
            Settings.bUsesPresence = false;
            Settings.bAllowJoinViaPresence = false;

            int32 QueryPort = 27015;
            FParse::Value(FCommandLine::Get(), TEXT("QueryPort="), QueryPort);

            Settings.Set(FName(TEXT("EEEUTTR")), FString("UnrealSteamTestLobbyEEEUTTR"), EOnlineDataAdvertisementType::ViaOnlineService);
            Settings.Set(FName(TEXT("MAX_FIT")), 3, EOnlineDataAdvertisementType::ViaOnlineService);

            SessionInterface->CreateSession(0, NAME_GameSession, Settings);
        }
    }
}

void APDLobbyGameMode::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
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
        int32 FreeSpace = MaxTeamSize - TeamCounts[i];
        if (FreeSpace > MaxFit) MaxFit = FreeSpace;
    }

    Settings->Set(FName(TEXT("MAX_FIT")), MaxFit, EOnlineDataAdvertisementType::ViaOnlineService);
    SessionInterface->UpdateSession(NAME_GameSession, *Settings);

    UE_LOG(LogTemp, Warning, TEXT("세션 메타데이터 갱신: MAX_FIT = %d"), MaxFit);
}
