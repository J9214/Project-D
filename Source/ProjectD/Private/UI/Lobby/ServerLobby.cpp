#include "UI/Lobby/ServerLobby.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "GameFramework/GameStateBase.h"
#include "PlayerState/PDPlayerState.h"

void UServerLobby::ApplyLobbyTeamInfos(const TArray<FTeamInfo>& InTeamInfos, ETeamType InLocalTeamID)
{
	const FTeamInfo* TeamAInfoData = nullptr;
	const FTeamInfo* TeamBInfoData = nullptr;
	const FTeamInfo* TeamCInfoData = nullptr;

	for (const FTeamInfo& TeamInfo : InTeamInfos)
	{
		if (TeamInfo.TeamID == ETeamType::TeamOne)
		{
			TeamAInfoData = &TeamInfo;
		}
		else if (TeamInfo.TeamID == ETeamType::TeamTwo)
		{
			TeamBInfoData = &TeamInfo;
		}
		else if (TeamInfo.TeamID == ETeamType::TeamThree)
		{
			TeamCInfoData = &TeamInfo;
		}
	}

	UpdateTeamInfoText(TeamAInfo, TEXT("A Team"), TeamAInfoData, InLocalTeamID == ETeamType::TeamOne);
	UpdateTeamInfoText(TeamBInfo, TEXT("B Team"), TeamBInfoData, InLocalTeamID == ETeamType::TeamTwo);
	UpdateTeamInfoText(TeamCInfo, TEXT("C Team"), TeamCInfoData, InLocalTeamID == ETeamType::TeamThree);

	UpdateLocalTeamPanels(InLocalTeamID);
}

void UServerLobby::UpdateTeamInfoText(
	UTextBlock* TextBlock,
	const TCHAR* TeamLabel,
	const FTeamInfo* TeamInfo,
	bool bIsMyTeam)
{
	if (!TextBlock)
	{
		return;
	}

	TextBlock->SetColorAndOpacity(
		bIsMyTeam
		? FSlateColor(FLinearColor::Yellow)
		: FSlateColor(FLinearColor::White));

	const int32 CurrentPlayerCount = TeamInfo ? TeamInfo->PlayerCount : 0;
	const int32 MaxPlayerCount = TeamInfo ? TeamInfo->MaxPlayerCount : 0;

	TextBlock->SetText(FText::FromString(
		FString::Printf(TEXT("%s : %d / %d"), TeamLabel, CurrentPlayerCount, MaxPlayerCount)));
}

void UServerLobby::CollectLocalTeamPlayerStates(ETeamType LocalTeamID, TArray<const APDPlayerState*>& OutTeamMembers) const
{
	OutTeamMembers.Reset();

	if (LocalTeamID == ETeamType::None || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyTeamPanel] Collect skipped LocalTeamID=%d"), static_cast<int32>(LocalTeamID));
		return;
	}

	const AGameStateBase* CurrentGameState = GetWorld()->GetGameState();
	if (!CurrentGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyTeamPanel] Collect skipped because GameState is null"));
		return;
	}

	const APDPlayerState* LocalPlayerState = GetOwningPlayer() ? GetOwningPlayer()->GetPlayerState<APDPlayerState>() : nullptr;

	for (APlayerState* BasePlayerState : CurrentGameState->PlayerArray)
	{
		const APDPlayerState* PDPlayerState = Cast<APDPlayerState>(BasePlayerState);
		if (!PDPlayerState || PDPlayerState->GetTeamID() != LocalTeamID)
		{
			continue;
		}

		OutTeamMembers.Add(PDPlayerState);
	}

	OutTeamMembers.Sort([LocalPlayerState](const APDPlayerState& A, const APDPlayerState& B)
	{
		const bool bAIsLocal = (&A == LocalPlayerState);
		const bool bBIsLocal = (&B == LocalPlayerState);
		if (bAIsLocal != bBIsLocal)
		{
			return bAIsLocal;
		}

		return A.GetPlayerId() < B.GetPlayerId();
	});
}

void UServerLobby::UpdateLocalTeamPanels(ETeamType LocalTeamID)
{
	TArray<const APDPlayerState*> LocalTeamMembers;
	CollectLocalTeamPlayerStates(LocalTeamID, LocalTeamMembers);

	const APDPlayerState* Slot0PlayerState = LocalTeamMembers.IsValidIndex(0) ? LocalTeamMembers[0] : nullptr;
	const APDPlayerState* Slot1PlayerState = LocalTeamMembers.IsValidIndex(1) ? LocalTeamMembers[1] : nullptr;

	const bool bHasSlot0 = Slot0PlayerState != nullptr;
	const bool bHasSlot1 = Slot1PlayerState != nullptr;

	if (TeamPanel_0)
	{
		TeamPanel_0->SetVisibility(bHasSlot0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (TeamPanel_1)
	{
		TeamPanel_1->SetVisibility(bHasSlot1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (TeamNickName_0)
	{
		TeamNickName_0->SetText(FText::FromString(bHasSlot0 ? Slot0PlayerState->GetResolvedDisplayName() : FString()));
	}

	if (TeamNickName_1)
	{
		TeamNickName_1->SetText(FText::FromString(bHasSlot1 ? Slot1PlayerState->GetResolvedDisplayName() : FString()));
	}

	if (bHasSlot0)
	{
		const FBPUniqueNetId AvatarUniqueNetId = Slot0PlayerState->GetAvatarUniqueNetId();
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[LobbyTeamPanel] Slot=0 Team=%d DisplayName=[%s] PlayerName=[%s] NetId=[%s] AvatarIdValid=%d"),
			static_cast<int32>(LocalTeamID),
			*Slot0PlayerState->GetDisplayName(),
			*Slot0PlayerState->GetPlayerName(),
			*Slot0PlayerState->GetUniqueId().ToString(),
			AvatarUniqueNetId.IsValid() ? 1 : 0);
		BP_UpdateTeamMemberAvatar(0, AvatarUniqueNetId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyTeamPanel] Slot=0 Team=%d Empty"), static_cast<int32>(LocalTeamID));
	}

	if (bHasSlot1)
	{
		const FBPUniqueNetId AvatarUniqueNetId = Slot1PlayerState->GetAvatarUniqueNetId();
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[LobbyTeamPanel] Slot=1 Team=%d DisplayName=[%s] PlayerName=[%s] NetId=[%s] AvatarIdValid=%d"),
			static_cast<int32>(LocalTeamID),
			*Slot1PlayerState->GetDisplayName(),
			*Slot1PlayerState->GetPlayerName(),
			*Slot1PlayerState->GetUniqueId().ToString(),
			AvatarUniqueNetId.IsValid() ? 1 : 0);
		BP_UpdateTeamMemberAvatar(1, AvatarUniqueNetId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyTeamPanel] Slot=1 Team=%d Empty"), static_cast<int32>(LocalTeamID));
	}
}

void UServerLobby::ApplyMatchStartServerTime(float InMatchStartServerTimeSec)
{
	MatchStartServerTimeSec = InMatchStartServerTimeSec;
	bHasMatchStartServerTime = true;

	StartMatchingTimeRefresh();
	RefreshMatchingTimeText();
}

void UServerLobby::NativeDestruct()
{
	StopMatchingTimeRefresh();
	Super::NativeDestruct();
}

void UServerLobby::RefreshMatchingTimeText()
{
	if (!MatchingTime || !bHasMatchStartServerTime || !GetWorld())
	{
		return;
	}

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState)
	{
		return;
	}

	const float ElapsedSec = FMath::Max(0.0f, GameState->GetServerWorldTimeSeconds() - MatchStartServerTimeSec);
	const int32 TotalSeconds = FMath::FloorToInt(ElapsedSec);
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	MatchingTime->SetText(FText::FromString(
		FString::Printf(TEXT("MatchingTime %d:%02d"), Minutes, Seconds)));
}

void UServerLobby::StartMatchingTimeRefresh()
{
	if (!bHasMatchStartServerTime || !GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(MatchingTimeRefreshHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MatchingTimeRefreshHandle,
		this,
		&UServerLobby::RefreshMatchingTimeText,
		1.0f,
		true);
}

void UServerLobby::StopMatchingTimeRefresh()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MatchingTimeRefreshHandle);
	}
}
