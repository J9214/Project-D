// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/PDLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "PlayerState/PDPlayerState.h"
#include "GameMode/PDLobbyGameMode.h"
#include "GameInstance/PDGameInstance.h"

void APDLobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController() == false)
    {
        return;
    }

    if (UIWidgetClass)
    {
        UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass);
        if (UIWidgetInstance)
        {
            UIWidgetInstance->AddToViewport();
            UIWidgetInstance->SetVisibility(ESlateVisibility::Visible);
        }
    }

    bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    SetInputMode(InputMode);
}

void APDLobbyPlayerController::ConnectToDedicatedServer()
{
    if (GetNetMode() == NM_Client)
    {
        UE_LOG(LogTemp, Warning, TEXT("Guest cannot start dedi search/join."));
        return;
    }

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem)
    {
        return;
    }

    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        return;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = 10000;
    SessionSearch->bIsLanQuery = false;

    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("n.VerifyPeerBuildUnitTests"));
    if (CVar)
    {
        CVar->Set(0);
    }

    SessionSearch->QuerySettings.Set(FName("GameFilter"), FString("UnrealSteamTestLobbyEEEUTTR"), EOnlineComparisonOp::Equals);

    SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &APDLobbyPlayerController::OnFindSessionsComplete)
    );

    UE_LOG(LogTemp, Warning, TEXT("=== 스팀 인터넷 세션 검색 시작 ==="));
    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void APDLobbyPlayerController::RequestTravelToLobby10()
{
    UE_LOG(LogTemp, Log, TEXT("[LobbyPC] RequestTravelToLobby10 called. Local=%d"), IsLocalController() ? 1 : 0);
    Server_RequestTravelToLobby10();
}

void APDLobbyPlayerController::Server_RequestTravelToLobby10_Implementation()
{
    APDLobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APDLobbyGameMode>() : nullptr;
    if (!LobbyGameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Server travel request failed: LobbyGameMode is null."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Server_RequestTravelToLobby10 received."));
    LobbyGameMode->TravelToLobby10();
}

void APDLobbyPlayerController::Server_SubmitDisplayName_Implementation(const FString& Name)
{
    if (auto* PS = GetPlayerState<APDPlayerState>())
    {
        PS->SetDisplayName(Name);

        if (APDLobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<APDLobbyGameMode>())
        {
            LobbyGameMode->BroadcastLobbyTeamInfos();
        }
    }
}

void APDLobbyPlayerController::Client_RequestDisplayName_Implementation()
{
    if (!IsLocalController())
    {
        return;
    }

    if (auto* GI = GetGameInstance<UPDGameInstance>())
    {
        Server_SubmitDisplayName(GI->GetPlayerLocalDisplayName());
    }
}

void APDLobbyPlayerController::Server_SubmitCharacterCustomInfo_Implementation(const FPDCharacterCustomInfo& CharacterInfo)
{
    APDPlayerState* PS = GetPlayerState<APDPlayerState>();
    if (!PS)
    {
        return;
    }

    PS->SetCharacterCustomInfo(CharacterInfo);
}

void APDLobbyPlayerController::Client_RequestCharacterCustomInfo_Implementation()
{
    if (!IsLocalController())
    {
        return;
    }

    auto* GI = GetGameInstance<UPDGameInstance>();
    if (!GI)
    {
        return;
    }

    Server_SubmitCharacterCustomInfo(GI->GetLocalCharacterCustomInfo());
}

void APDLobbyPlayerController::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (bWasSuccessful && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== 검색 완료: 총 %d개의 세션을 찾았습니다. ==="), SessionSearch->SearchResults.Num());
        CurrentSessionIndex = 0;
        TryJoinNextAvailableSession();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("검색된 서버가 없습니다."));
    }
}

void APDLobbyPlayerController::TryJoinNextAvailableSession()
{
    if (GetNetMode() < NM_Client)
    {
        MyTeamSize = GetWorld()->GetNumPlayerControllers();
    }
    else
    {
        MyTeamSize = 1;
    }

    if (CurrentSessionIndex >= SessionSearch->SearchResults.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("적합한 빈자리가 있는 서버를 찾지 못했습니다."));
        return;
    }

    auto& SelectedSession = SessionSearch->SearchResults[CurrentSessionIndex];

    int32 MaxFitSize = 0;
    SelectedSession.Session.SessionSettings.Get(FName("MAX_FIT"), MaxFitSize);

    if (MyTeamSize > MaxFitSize)
    {
        CurrentSessionIndex++;
        TryJoinNextAvailableSession();
        return;
    }

    APDPlayerState* PDPS = GetPlayerState<APDPlayerState>();
    if (!PDPS)
    {
        return;
    }

    const FString NetIdKey = PDPS->GetUniqueId().ToString();

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FString ConnectString;
            if (SessionInterface->GetResolvedConnectString(SelectedSession, NAME_GamePort, ConnectString))
            {
                PendingDediUrl = FString::Printf(TEXT("%s?LeaderSteamId=%s&TeamSize=%d"),*ConnectString, *NetIdKey, MyTeamSize);

                UE_LOG(LogTemp, Log, TEXT("Steam Connect String: %s"), *PendingDediUrl);
            }
        }
    }

    if (GetNetMode() < NM_Client)
    {
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APDLobbyPlayerController* PlayerController = Cast<APDLobbyPlayerController>(It->Get());
            if (PlayerController && PlayerController != this)
            {
                UE_LOG(LogTemp, Log, TEXT("Steam Connect String: %s"), *PendingDediUrl);
                PlayerController->Client_BeginTravelToDedi(PendingDediUrl);
            }
        }
    }

    Client_BeginTravelToDedi(PendingDediUrl);
}

void APDLobbyPlayerController::Client_BeginTravelToDedi_Implementation(const FString& DediUrl)
{
    PendingDediUrl = DediUrl;

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    IOnlineSessionPtr SessionInterface = Subsystem ? Subsystem->GetSessionInterface() : nullptr;

    if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession))
    {
        DestroyHandleForDedi = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &APDLobbyPlayerController::OnDestroySessionComplete)
        );

        UE_LOG(LogTemp, Warning, TEXT("로컬 세션(GameSession) 파괴 후 데디 이동 시작"));
        SessionInterface->DestroySession(NAME_GameSession);
        return;
    }

    ClientTravel(PendingDediUrl, TRAVEL_Absolute);
}

void APDLobbyPlayerController::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    IOnlineSessionPtr SessionInterface = Subsystem ? Subsystem->GetSessionInterface() : nullptr;

    if (SessionInterface.IsValid() && DestroyHandleForDedi.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandleForDedi);
        DestroyHandleForDedi.Reset();
    }

    ClientTravel(PendingDediUrl, TRAVEL_Absolute);
}
