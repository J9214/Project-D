// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/PDGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "CreateSessionCallbackProxyAdvanced.h" 
#include "AdvancedSessionsLibrary.h"


void UPDGameInstance::Init()
{
    Super::Init();

    BindSessionDelegates();
}

void UPDGameInstance::BindSessionDelegates()
{
    if (IsRunningDedicatedServer())
    {
        UE_LOG(LogTemp, Error, TEXT("DediServer!"));
        return;
    }

    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub)
    {
        UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem Get Failed"));
        return;
    }

    SessionInterface = OnlineSub->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem GetSessionInterface Failed"));
        return;
    }

    InviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
        FOnSessionUserInviteAcceptedDelegate::CreateUObject(
            this,
            &UPDGameInstance::HandleInviteAccepted
        )
    );

    JoinCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(
            this,
            &UPDGameInstance::HandleJoinSessionComplete
        )
    );

    DestroyCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateUObject(
            this,
            &UPDGameInstance::HandleDestroySessionComplete
        )
    );
}

void UPDGameInstance::HandleInviteAccepted(const bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId>, const FOnlineSessionSearchResult& InviteResult)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("InviteAccepted Failed"));
        return;
    }

    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface IsUnValid"));
        return;
    }

    SessionInterface->JoinSession(ControllerId, NAME_GameSession, InviteResult);
}

void UPDGameInstance::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result != EOnJoinSessionCompleteResult::Success || !SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("JoinSession Failed"));
        return;
    }

    FString ConnectString;

    if (!SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        UE_LOG(LogTemp, Error, TEXT("GetResolvedConnectString Failed"));
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("HandleJoinSessionComplete GetPlayerController Failed"));
        return;
    }

    int32 ColonIndex;
    if (ConnectString.FindLastChar(':', ColonIndex))
    {
        ConnectString = ConnectString.Left(ColonIndex);
    }

    ConnectString = ConnectString.Replace(TEXT("steam:"), TEXT("steam."));
    if (!ConnectString.StartsWith(TEXT("steam.")))
    {
        ConnectString = FString::Printf(TEXT("steam.%s"), *ConnectString);
    }

    PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
}

void UPDGameInstance::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionName != NAME_GameSession)
    {
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("ClientTravelTo GetPlayerController Failed"));
        return;
    }

    if (SessionInterface.IsValid() && DestroyCompleteHandle.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroyCompleteHandle);
        DestroyCompleteHandle.Reset();
    }

    if (bPendingTravelToDedi)
    {
        bPendingTravelToDedi = false;
        PlayerController->ClientTravel(PendingTravelURL, TRAVEL_Absolute);
    }
}

void UPDGameInstance::HostPartySessionCreate()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("HostPartySessionCreate GetPlayerController Failed"));
        return;
    }

    TArray<FSessionPropertyKeyPair> ExtraSettings;

    CreateSessionProxy = UCreateSessionCallbackProxyAdvanced::CreateAdvancedSession(
        this,
        ExtraSettings,
        PlayerController,
        3,      // Public
        0,      // Private
        false,  // UseLAN
        true,   // AllowInvites
        false,  // IsDedicatedServer
        true,   // UseLobbiesIfAvailable
        true,   // AllowJoinViaPresence
        false,  // AllowJoinViaPresenceFriendsOnly
        false,  // AntiCheatProtected
        false,  // UsesStats
        true,   // ShouldAdvertise
        false,  // UseLobbiesVoiceChatIfAvailable
        true    // StartAfterCreat
    );

    CreateSessionProxy->OnSuccess.AddDynamic(this, &UPDGameInstance::OnCreateHostSessionSuccess);
    CreateSessionProxy->OnFailure.AddDynamic(this, &UPDGameInstance::OnCreateHostSessionFailure);
    CreateSessionProxy->Activate();
}

void UPDGameInstance::OnCreateHostSessionSuccess()
{
    FString Options = TEXT("listen?bIsLanMatch=0?NetDriverClassName=SteamNetDriver");
    UE_LOG(LogTemp, Warning, TEXT("Lobby Move Start with Options: %s"), *Options);
    UGameplayStatics::OpenLevel(GetWorld(), FName("/Game/ProjectD/Maps/FriendsLobbyLevel"), true, Options);
}

void UPDGameInstance::OnCreateHostSessionFailure()
{
    UE_LOG(LogTemp, Warning, TEXT("SessionCreateFail"));
}

void UPDGameInstance::TravelToDedicated(const FString& DediAddress)
{

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("ClientTravelTo GetPlayerController Failed"));
        return;
    }

    PendingTravelURL = DediAddress;
    bPendingTravelToDedi = true;

    PlayerController->ClientTravel(PendingTravelURL, TRAVEL_Absolute);
}