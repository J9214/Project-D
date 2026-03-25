#include "UI/Lobby/ServerLobby.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "GameFramework/GameStateBase.h"

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

	const FTeamInfo* LocalTeamInfo = nullptr;
	if (InLocalTeamID == ETeamType::TeamOne)
	{
		LocalTeamInfo = TeamAInfoData;
	}
	else if (InLocalTeamID == ETeamType::TeamTwo)
	{
		LocalTeamInfo = TeamBInfoData;
	}
	else if (InLocalTeamID == ETeamType::TeamThree)
	{
		LocalTeamInfo = TeamCInfoData;
	}

	UpdateLocalTeamPanels(LocalTeamInfo);
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

void UServerLobby::UpdateLocalTeamPanels(const FTeamInfo* TeamInfo)
{
	const bool bHasSlot0 = TeamInfo && TeamInfo->bHasTeamMember_0;
	const bool bHasSlot1 = TeamInfo && TeamInfo->bHasTeamMember_1;

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
		TeamNickName_0->SetText(FText::FromString(bHasSlot0 ? TeamInfo->TeamMemberDisplayName_0 : FString()));
	}

	if (TeamNickName_1)
	{
		TeamNickName_1->SetText(FText::FromString(bHasSlot1 ? TeamInfo->TeamMemberDisplayName_1 : FString()));
	}

	if (bHasSlot0)
	{
		BP_UpdateTeamMemberAvatar(0, TeamInfo->TeamMemberId_0);
	}

	if (bHasSlot1)
	{
		BP_UpdateTeamMemberAvatar(1, TeamInfo->TeamMemberId_1);
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
