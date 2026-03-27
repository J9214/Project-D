// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/PDServerLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "PlayerState/PDPlayerState.h"
#include "UI/Lobby/ServerLobby.h"
#include "GameMode/PDLobbyGameMode.h"

void APDServerLobbyPlayerController::BeginPlay()
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
            ServerLobbyWidget = Cast<UServerLobby>(UIWidgetInstance);
            UIWidgetInstance->AddToViewport();
            UIWidgetInstance->SetVisibility(ESlateVisibility::Visible);
            RefreshLobbyWidget();
        }
    }

    bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    SetInputMode(InputMode);
}

void APDServerLobbyPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    const APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>();
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[LobbyClientController] OnRep_PlayerState Team=%d PlayerName=[%s] DisplayName=[%s] Resolved=[%s] NetId=[%s]"),
        PDPlayerState ? static_cast<int32>(PDPlayerState->GetTeamID()) : static_cast<int32>(ETeamType::None),
        PDPlayerState ? *PDPlayerState->GetPlayerName() : TEXT("None"),
        PDPlayerState ? *PDPlayerState->GetDisplayName() : TEXT("None"),
        PDPlayerState ? *PDPlayerState->GetResolvedDisplayName() : TEXT("None"),
        PDPlayerState ? *PDPlayerState->GetUniqueId().ToString() : TEXT("None"));

    RefreshLobbyWidget();
}

void APDServerLobbyPlayerController::RequestTravelToLobby10()
{
    UE_LOG(LogTemp, Log, TEXT("[ServerLobbyPC] RequestTravelToLobby10 called. Local=%d"), IsLocalController() ? 1 : 0);
    Server_RequestTravelToLobby10();
}

void APDServerLobbyPlayerController::Server_RequestTravelToLobby10_Implementation()
{
    APDLobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APDLobbyGameMode>() : nullptr;
    if (!LobbyGameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ServerLobbyPC] Travel request failed: LobbyGameMode is null."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[ServerLobbyPC] Server_RequestTravelToLobby10 received."));
    LobbyGameMode->TravelToLobby10();
}

void APDServerLobbyPlayerController::Client_UpdateLobbyTeamInfos_Implementation(
    const TArray<FTeamInfo>& InTeamInfos,
    const float InMatchStartServerTimeSec)
{
    CachedTeamInfos = InTeamInfos;
    CachedMatchStartServerTimeSec = InMatchStartServerTimeSec;
    bHasCachedMatchStartServerTime = true;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[LobbyClientController] Client_UpdateLobbyTeamInfos TeamCount=%d MatchStart=%.2f"),
        CachedTeamInfos.Num(),
        CachedMatchStartServerTimeSec);

    RefreshLobbyWidget();
}

void APDServerLobbyPlayerController::RefreshLobbyWidget()
{
    if (!ServerLobbyWidget)
    {
        return;
    }

    const APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>();
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[LobbyClientController] RefreshLobbyWidget Team=%d PlayerName=[%s] DisplayName=[%s] Resolved=[%s] NetId=[%s]"),
        PDPlayerState ? static_cast<int32>(PDPlayerState->GetTeamID()) : static_cast<int32>(ETeamType::None),
        PDPlayerState ? *PDPlayerState->GetPlayerName() : TEXT("None"),
        PDPlayerState ? *PDPlayerState->GetDisplayName() : TEXT("None"),
        PDPlayerState ? *PDPlayerState->GetResolvedDisplayName() : TEXT("None"),
        PDPlayerState ? *PDPlayerState->GetUniqueId().ToString() : TEXT("None"));

    ServerLobbyWidget->ApplyLobbyTeamInfos(CachedTeamInfos, GetLocalTeamID());
    if (bHasCachedMatchStartServerTime)
    {
        ServerLobbyWidget->ApplyMatchStartServerTime(CachedMatchStartServerTimeSec);
    }
}

ETeamType APDServerLobbyPlayerController::GetLocalTeamID() const
{
    const APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>();
    return PDPlayerState ? PDPlayerState->GetTeamID() : ETeamType::None;
}
