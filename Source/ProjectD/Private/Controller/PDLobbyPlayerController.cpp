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
    if (CVar) CVar->Set(0);

    SessionSearch->QuerySettings.Set(FName("GameFilter"), FString("UnrealSteamTestLobbyEEEUTTR"), EOnlineComparisonOp::Equals);

    SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &APDLobbyPlayerController::OnFindSessionsComplete)
    );

    UE_LOG(LogTemp, Warning, TEXT("=== 스팀 인터넷 세션 검색 시작 ==="));
    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void APDLobbyPlayerController::Client_TravelToTargetServer_Implementation(const FString& ConnectString)
{
    FString FinalURL = ConnectString + TEXT("?NetDriverClassName=OnlineSubsystemUtils.IpNetDriver");
    ClientTravel(FinalURL, TRAVEL_Absolute);
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
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

    int32 MaxFitSize = 0;
    SelectedSession.Session.SessionSettings.Get(FName("MAX_FIT"), MaxFitSize);

    if (MyTeamSize <= MaxFitSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("%d번 서버 접속 시도... (자리 여유 있음)"), CurrentSessionIndex);

        SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(this, &APDLobbyPlayerController::OnJoinSessionComplete)
        );

        SessionInterface->JoinSession(0, NAME_GameSession, SelectedSession);
    }
    else
    {
        CurrentSessionIndex++;
        TryJoinNextAvailableSession();
    }
}

void APDLobbyPlayerController::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        CurrentSessionIndex++;
        TryJoinNextAvailableSession();
        return;
    }

    IOnlineSessionPtr SessionInterface = IOnlineSubsystem::Get()->GetSessionInterface();
    FString ConnectString;

    if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        PendingSteamAddress = ConnectString + FString::Printf(TEXT("?TeamSize=%d"), MyTeamSize);

        if (GetNetMode() < NM_Client)
        {
            for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                APDLobbyPlayerController* PlayerController = Cast<APDLobbyPlayerController>(It->Get());
                if (PlayerController && PlayerController != this)
                {
                    PlayerController->Client_TravelToTargetServer(PendingSteamAddress);
                }
            }
        }

        if (SessionInterface->GetNamedSession(NAME_GameSession))
        {
            SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
                FOnDestroySessionCompleteDelegate::CreateUObject(this, &APDLobbyPlayerController::OnDestroySessionComplete)
            );
            SessionInterface->DestroySession(NAME_GameSession);
        }
        else
        {
            OnDestroySessionComplete(NAME_GameSession, true);
        }
    }
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

void APDLobbyPlayerController::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("리슨 서버 세션 파괴 완료. 데디 서버로 이동 시도..."));

    if (!PendingSteamAddress.IsEmpty())
    {
        FString FinalURL = PendingSteamAddress;
        FinalURL += TEXT("?NetDriverClassName=OnlineSubsystemUtils.IpNetDriver");

        UE_LOG(LogTemp, Error, TEXT("최종 이동 URL: %s"), *FinalURL);
        ClientTravel(FinalURL, TRAVEL_Absolute);

        PendingSteamAddress = TEXT("");
    }
}