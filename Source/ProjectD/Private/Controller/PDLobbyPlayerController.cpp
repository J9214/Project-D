// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/PDLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

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

void APDLobbyPlayerController::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (bWasSuccessful && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
    {
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

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FString ConnectString;
            if (SessionInterface->GetResolvedConnectString(SelectedSession, NAME_GamePort, ConnectString))
            {
                PendingDediUrl = FString::Printf(TEXT("%s?TeamSize=%d"), *ConnectString, MyTeamSize);

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