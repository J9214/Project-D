// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/PDGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "CreateSessionCallbackProxyAdvanced.h" 
#include "AdvancedSessionsLibrary.h"
#include "Controller/PDLobbyPlayerController.h"
#include "Controller/PDPlayerController.h"
#include "Controller/PDServerLobbyPlayerController.h"
#include "PlayerState/PDPlayerState.h"

namespace
{
FString DescribeCharacterCustomInfo(const FPDCharacterCustomInfo& CharacterCustomInfo)
{
	FString Summary = FString::Printf(
		TEXT("CharacterId=%d Enum=%d Float=%d Color=%d Bool=%d"),
		CharacterCustomInfo.CharacterId,
		CharacterCustomInfo.EnumParameters.Num(),
		CharacterCustomInfo.FloatParameters.Num(),
		CharacterCustomInfo.ColorParameters.Num(),
		CharacterCustomInfo.BoolParameters.Num());

	for (int32 Index = 0; Index < CharacterCustomInfo.EnumParameters.Num(); ++Index)
	{
		const FPDMutableEnumParameter& Param = CharacterCustomInfo.EnumParameters[Index];
		Summary += FString::Printf(
			TEXT(" | Enum[%d]={Name='%s',Option='%s',Range=%d}"),
			Index,
			*Param.ParameterName,
			*Param.SelectedOptionName,
			Param.RangeIndex);
	}

	for (int32 Index = 0; Index < CharacterCustomInfo.FloatParameters.Num(); ++Index)
	{
		const FPDMutableFloatParameter& Param = CharacterCustomInfo.FloatParameters[Index];
		Summary += FString::Printf(
			TEXT(" | Float[%d]={Name='%s',Value=%.3f,Range=%d}"),
			Index,
			*Param.ParameterName,
			Param.Value,
			Param.RangeIndex);
	}

	for (int32 Index = 0; Index < CharacterCustomInfo.ColorParameters.Num(); ++Index)
	{
		const FPDMutableColorParameter& Param = CharacterCustomInfo.ColorParameters[Index];
		Summary += FString::Printf(
			TEXT(" | Color[%d]={Name='%s',Value=%s}"),
			Index,
			*Param.ParameterName,
			*Param.Value.ToString());
	}

	for (int32 Index = 0; Index < CharacterCustomInfo.BoolParameters.Num(); ++Index)
	{
		const FPDMutableBoolParameter& Param = CharacterCustomInfo.BoolParameters[Index];
		Summary += FString::Printf(
			TEXT(" | Bool[%d]={Name='%s',Value=%d}"),
			Index,
			*Param.ParameterName,
			Param.Value ? 1 : 0);
	}

	return Summary;
}
}


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
	UE_LOG(LogTemp, Warning, TEXT("[CharacterCustomizationFlow][GameInstance::SetLocalCharacterCustomInfo] %s"), *DescribeCharacterCustomInfo(CharacterCustomInfo));
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

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[CharacterCustomizationFlow][GameInstance::TrySubmitCharacterCustomInfo] PC=%s HasAuthority=%d NetMode=%d %s"),
		*GetNameSafe(PC),
		PC->HasAuthority() ? 1 : 0,
		static_cast<int32>(World->GetNetMode()),
		*DescribeCharacterCustomInfo(LocalCharacterCustomInfo));

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

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[CharacterCustomizationFlow][GameInstance::SubmitThroughController] Route=AuthorityDirect PC=%s PS=%s %s"),
			*GetNameSafe(PlayerController),
			*GetNameSafe(PlayerState),
			*DescribeCharacterCustomInfo(LocalCharacterCustomInfo));
        PlayerState->SetCharacterCustomInfo(LocalCharacterCustomInfo);
        return true;
    }

    if (APDLobbyPlayerController* LobbyPC = Cast<APDLobbyPlayerController>(PlayerController))
    {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[CharacterCustomizationFlow][GameInstance::SubmitThroughController] Route=LobbyPC PC=%s %s"),
			*GetNameSafe(LobbyPC),
			*DescribeCharacterCustomInfo(LocalCharacterCustomInfo));
        LobbyPC->Server_SubmitCharacterCustomInfo(LocalCharacterCustomInfo);
        return true;
    }

    if (APDServerLobbyPlayerController* ServerLobbyPC = Cast<APDServerLobbyPlayerController>(PlayerController))
    {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[CharacterCustomizationFlow][GameInstance::SubmitThroughController] Route=ServerLobbyPC PC=%s %s"),
			*GetNameSafe(ServerLobbyPC),
			*DescribeCharacterCustomInfo(LocalCharacterCustomInfo));
        ServerLobbyPC->Server_SubmitCharacterCustomInfo(LocalCharacterCustomInfo);
        return true;
    }

    if (APDPlayerController* GamePC = Cast<APDPlayerController>(PlayerController))
    {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[CharacterCustomizationFlow][GameInstance::SubmitThroughController] Route=GamePC PC=%s %s"),
			*GetNameSafe(GamePC),
			*DescribeCharacterCustomInfo(LocalCharacterCustomInfo));
        GamePC->Server_SubmitCharacterCustomInfo(LocalCharacterCustomInfo);
        return true;
    }

    return false;
}
