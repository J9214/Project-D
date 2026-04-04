// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/PDGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "CreateSessionCallbackProxyAdvanced.h" 
#include "AdvancedSessionsLibrary.h"
#include "Controller/PDLobbyPlayerController.h"
#include "Controller/PDPlayerController.h"
#include "Controller/PDServerLobbyPlayerController.h"
#include "PlayerState/PDPlayerState.h"

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

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("HandleJoinSessionComplete GetPlayerController Failed"));
        return;
    }

    FString ConnectString;

    if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        int32 ColonIndex;
        if (ConnectString.FindChar(':', ColonIndex))
        {
            ConnectString = ConnectString.Left(ColonIndex);
        }

        if (!ConnectString.StartsWith(TEXT("steam.")))
        {
            ConnectString = FString::Printf(TEXT("steam.%s"), *ConnectString);
        }

        PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
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
    FString Options = TEXT("listen");
    UE_LOG(LogTemp, Warning, TEXT("Lobby Move Start with Options: %s"), *Options);
    UGameplayStatics::OpenLevel(GetWorld(), FName("/Game/ProjectD/Maps/FriendsLobbyLevel"), true, Options);
}

void UPDGameInstance::OnCreateHostSessionFailure()
{
    UE_LOG(LogTemp, Warning, TEXT("SessionCreateFail"));
}

void UPDGameInstance::SetLocalCharacterCustomInfo(const FPDCharacterCustomInfo& CharacterCustomInfo)
{
    LocalCharacterCustomInfo = CharacterCustomInfo;

	TrySubmitCharacterCustomInfo();
}

void UPDGameInstance::TrySubmitCharacterCustomInfo()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

    if (SubmitCharacterCustomInfoThroughController(PC))
    {
        return;
    }

    if (World->GetNetMode() == NM_Client && PC->GetNetConnection() == nullptr)
    {
        return;
    }
}

bool UPDGameInstance::SubmitCharacterCustomInfoThroughController(APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return false;
    }

    APDPlayerState* PlayerState = PlayerController->GetPlayerState<APDPlayerState>();
    if (PlayerController->HasAuthority())
    {
        if (!PlayerState)
        {
            return false;
        }

        PlayerState->SetCharacterCustomInfo(LocalCharacterCustomInfo);
        return true;
    }

    if (APDLobbyPlayerController* LobbyPC = Cast<APDLobbyPlayerController>(PlayerController))
    {
        LobbyPC->Server_SubmitCharacterCustomInfo(LocalCharacterCustomInfo);
        return true;
    }

    if (APDServerLobbyPlayerController* ServerLobbyPC = Cast<APDServerLobbyPlayerController>(PlayerController))
    {
        ServerLobbyPC->Server_SubmitCharacterCustomInfo(LocalCharacterCustomInfo);
        return true;
    }

    if (APDPlayerController* GamePC = Cast<APDPlayerController>(PlayerController))
    {
        GamePC->Server_SubmitCharacterCustomInfo(LocalCharacterCustomInfo);
        return true;
    }

    return false;
}
