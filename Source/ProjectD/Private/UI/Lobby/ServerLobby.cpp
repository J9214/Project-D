#include "UI/Lobby/ServerLobby.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "GameFramework/GameStateBase.h"
#include "PlayerState/PDPlayerState.h"

void UServerLobby::ApplyLobbyTeamInfos(const TArray<FTeamInfo>& InTeamInfos, ETeamType InLocalTeamID)
{
	CachedTeamInfos = InTeamInfos;
	CachedLocalTeamID = InLocalTeamID;
	LobbyTeamRefreshRetryCount = 0;
	RebindCharacterCustomInfoDelegates();
	RefreshLobbyTeamInfos();
}

void UServerLobby::RefreshLobbyTeamInfos()
{
	const TArray<FTeamInfo>& InTeamInfos = CachedTeamInfos;
	const ETeamType InLocalTeamID = CachedLocalTeamID;

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
	UpdateOtherTeamAvatars(InLocalTeamID);

	if (AreAllExpectedPlayerStatesReady())
	{
		StopLobbyTeamRefreshRetry();
		return;
	}

	StartLobbyTeamRefreshRetry();
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

	if (LocalPlayerState)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[LobbyClientLocalPlayer] Team=%d PlayerName=[%s] DisplayName=[%s] Resolved=[%s] NetId=[%s]"),
			static_cast<int32>(LocalPlayerState->GetTeamID()),
			*LocalPlayerState->GetPlayerName(),
			*LocalPlayerState->GetDisplayName(),
			*LocalPlayerState->GetResolvedDisplayName(),
			*LocalPlayerState->GetUniqueId().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyClientLocalPlayer] LocalPlayerState is null"));
	}

	for (APlayerState* BasePlayerState : CurrentGameState->PlayerArray)
	{
		const APDPlayerState* PDPlayerState = Cast<APDPlayerState>(BasePlayerState);
		if (!PDPlayerState)
		{
			continue;
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[LobbyClientPlayerState] Team=%d PlayerName=[%s] DisplayName=[%s] Resolved=[%s] NetId=[%s] IsLocal=%d"),
			static_cast<int32>(PDPlayerState->GetTeamID()),
			*PDPlayerState->GetPlayerName(),
			*PDPlayerState->GetDisplayName(),
			*PDPlayerState->GetResolvedDisplayName(),
			*PDPlayerState->GetUniqueId().ToString(),
			PDPlayerState == LocalPlayerState ? 1 : 0);

		if (PDPlayerState->GetTeamID() != LocalTeamID)
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

void UServerLobby::CollectTeamPlayerStates(ETeamType TeamID, TArray<const APDPlayerState*>& OutTeamMembers) const
{
	OutTeamMembers.Reset();

	if (TeamID == ETeamType::None || !GetWorld())
	{
		return;
	}

	const AGameStateBase* CurrentGameState = GetWorld()->GetGameState();
	if (!CurrentGameState)
	{
		return;
	}

	for (APlayerState* BasePlayerState : CurrentGameState->PlayerArray)
	{
		const APDPlayerState* PDPlayerState = Cast<APDPlayerState>(BasePlayerState);
		if (!PDPlayerState || PDPlayerState->GetTeamID() != TeamID)
		{
			continue;
		}

		OutTeamMembers.Add(PDPlayerState);
	}

	OutTeamMembers.Sort([](const APDPlayerState& A, const APDPlayerState& B)
	{
		return A.GetPlayerId() < B.GetPlayerId();
	});
}

void UServerLobby::UpdateLocalTeamPanels(ETeamType LocalTeamID)
{
	TArray<const APDPlayerState*> LocalTeamMembers;
	CollectLocalTeamPlayerStates(LocalTeamID, LocalTeamMembers);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[LobbyTeamPanel] Refresh Team=%d MemberCount=%d"),
		static_cast<int32>(LocalTeamID),
		LocalTeamMembers.Num());

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
		NotifyAvatarTarget(ELobbyAvatarTarget::LocalTeam, 0, Slot0PlayerState, LocalTeamID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyTeamPanel] Slot=0 Team=%d Empty"), static_cast<int32>(LocalTeamID));
		BP_UpdateLobbyMemberAvatar(ELobbyAvatarTarget::LocalTeam, 0, false, FBPUniqueNetId(), FPDCharacterCustomInfo());
	}

	if (bHasSlot1)
	{
		NotifyAvatarTarget(ELobbyAvatarTarget::LocalTeam, 1, Slot1PlayerState, LocalTeamID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyTeamPanel] Slot=1 Team=%d Empty"), static_cast<int32>(LocalTeamID));
		BP_UpdateLobbyMemberAvatar(ELobbyAvatarTarget::LocalTeam, 1, false, FBPUniqueNetId(), FPDCharacterCustomInfo());
	}
}

void UServerLobby::NotifyAvatarTarget(ELobbyAvatarTarget AvatarTarget, int32 SlotIndex, const APDPlayerState* SlotPlayerState, ETeamType SourceTeamID)
{
	if (!SlotPlayerState)
	{
		BP_UpdateLobbyMemberAvatar(AvatarTarget, SlotIndex, false, FBPUniqueNetId(), FPDCharacterCustomInfo());
		return;
	}

	const FBPUniqueNetId AvatarUniqueNetId = SlotPlayerState->GetAvatarUniqueNetId();
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[LobbyAvatarTarget] Target=%d SourceTeam=%d Slot=%d DisplayName=[%s] PlayerName=[%s] Resolved=[%s] NetId=[%s] AvatarIdValid=%d"),
		static_cast<int32>(AvatarTarget),
		static_cast<int32>(SourceTeamID),
		SlotIndex,
		*SlotPlayerState->GetDisplayName(),
		*SlotPlayerState->GetPlayerName(),
		*SlotPlayerState->GetResolvedDisplayName(),
		*SlotPlayerState->GetUniqueId().ToString(),
		AvatarUniqueNetId.IsValid() ? 1 : 0);

	BP_UpdateLobbyMemberAvatar(AvatarTarget, SlotIndex, true, AvatarUniqueNetId, SlotPlayerState->GetCharacterCustomInfo());
}

void UServerLobby::UpdateOtherTeamAvatars(ETeamType LocalTeamID)
{
	TArray<const APDPlayerState*> TeamMembers;
	TArray<ETeamType> OtherTeams;
	OtherTeams.Reserve(2);

	for (int32 TeamIndex = 0; TeamIndex < static_cast<int32>(ETeamType::MAX); ++TeamIndex)
	{
		const ETeamType CandidateTeam = static_cast<ETeamType>(TeamIndex);
		if (CandidateTeam == LocalTeamID)
		{
			continue;
		}

		OtherTeams.Add(CandidateTeam);
	}

	for (int32 OtherTeamIndex = 0; OtherTeamIndex < 2; ++OtherTeamIndex)
	{
		const ELobbyAvatarTarget AvatarTarget = (OtherTeamIndex == 0) ? ELobbyAvatarTarget::OtherTeamB : ELobbyAvatarTarget::OtherTeamC;
		const ETeamType SourceTeamID = OtherTeams.IsValidIndex(OtherTeamIndex) ? OtherTeams[OtherTeamIndex] : ETeamType::None;

		if (SourceTeamID == ETeamType::None)
		{
			BP_UpdateLobbyMemberAvatar(AvatarTarget, 0, false, FBPUniqueNetId(), FPDCharacterCustomInfo());
			BP_UpdateLobbyMemberAvatar(AvatarTarget, 1, false, FBPUniqueNetId(), FPDCharacterCustomInfo());
			continue;
		}

		CollectTeamPlayerStates(SourceTeamID, TeamMembers);

		for (int32 SlotIndex = 0; SlotIndex < 2; ++SlotIndex)
		{
			const APDPlayerState* SlotPlayerState = TeamMembers.IsValidIndex(SlotIndex) ? TeamMembers[SlotIndex] : nullptr;
			if (!SlotPlayerState)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[LobbyOtherTeamAvatar] Target=%d SourceTeam=%d Slot=%d Empty"),
					static_cast<int32>(AvatarTarget),
					static_cast<int32>(SourceTeamID),
					SlotIndex);
				BP_UpdateLobbyMemberAvatar(AvatarTarget, SlotIndex, false, FBPUniqueNetId(), FPDCharacterCustomInfo());
				continue;
			}

			NotifyAvatarTarget(AvatarTarget, SlotIndex, SlotPlayerState, SourceTeamID);
		}
	}
}

int32 UServerLobby::GetExpectedReadyPlayerCount(ETeamType TeamID) const
{
	for (const FTeamInfo& TeamInfo : CachedTeamInfos)
	{
		if (TeamInfo.TeamID == TeamID)
		{
			return TeamInfo.PlayerCount;
		}
	}

	return 0;
}

int32 UServerLobby::CountReadyPlayerStates(ETeamType TeamID) const
{
	if (TeamID == ETeamType::None || !GetWorld())
	{
		return 0;
	}

	const AGameStateBase* CurrentGameState = GetWorld()->GetGameState();
	if (!CurrentGameState)
	{
		return 0;
	}

	int32 ReadyPlayerCount = 0;

	for (APlayerState* BasePlayerState : CurrentGameState->PlayerArray)
	{
		const APDPlayerState* PDPlayerState = Cast<APDPlayerState>(BasePlayerState);
		if (!PDPlayerState || PDPlayerState->GetTeamID() != TeamID)
		{
			continue;
		}

		const bool bHasReadyDisplayName = !PDPlayerState->GetResolvedDisplayName().IsEmpty();
		const bool bHasReadyUniqueId = PDPlayerState->GetAvatarUniqueNetId().IsValid();
		if (bHasReadyDisplayName && bHasReadyUniqueId)
		{
			++ReadyPlayerCount;
		}
	}

	return ReadyPlayerCount;
}

bool UServerLobby::AreAllExpectedPlayerStatesReady() const
{
	if (CachedTeamInfos.IsEmpty())
	{
		return true;
	}

	bool bAllReady = true;

	for (const FTeamInfo& TeamInfo : CachedTeamInfos)
	{
		const int32 ReadyPlayerCount = CountReadyPlayerStates(TeamInfo.TeamID);
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[LobbyTeamRefreshRetry] Team=%d Ready=%d Expected=%d RetryCount=%d"),
			static_cast<int32>(TeamInfo.TeamID),
			ReadyPlayerCount,
			TeamInfo.PlayerCount,
			LobbyTeamRefreshRetryCount);

		if (ReadyPlayerCount < TeamInfo.PlayerCount)
		{
			bAllReady = false;
		}
	}

	return bAllReady;
}

void UServerLobby::RetryRefreshLobbyTeamInfos()
{
	++LobbyTeamRefreshRetryCount;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[LobbyTeamRefreshRetry] RetryRefreshLobbyTeamInfos Attempt=%d"),
		LobbyTeamRefreshRetryCount);

	RebindCharacterCustomInfoDelegates();
	RefreshLobbyTeamInfos();
}

void UServerLobby::RebindCharacterCustomInfoDelegates()
{
	UnbindCharacterCustomInfoDelegates();

	if (!GetWorld())
	{
		return;
	}

	const AGameStateBase* CurrentGameState = GetWorld()->GetGameState();
	if (!CurrentGameState)
	{
		return;
	}

	for (APlayerState* BasePlayerState : CurrentGameState->PlayerArray)
	{
		APDPlayerState* PDPlayerState = Cast<APDPlayerState>(BasePlayerState);
		if (!PDPlayerState)
		{
			continue;
		}

		const FDelegateHandle Handle = PDPlayerState->OnCharacterCustomInfoChangedNative().AddUObject(
			this,
			&ThisClass::HandleLobbyPlayerCharacterCustomInfoChanged);
		CharacterCustomInfoChangedHandles.Add(PDPlayerState, Handle);
	}
}

void UServerLobby::UnbindCharacterCustomInfoDelegates()
{
	for (TPair<TWeakObjectPtr<APDPlayerState>, FDelegateHandle>& Pair : CharacterCustomInfoChangedHandles)
	{
		if (APDPlayerState* PDPlayerState = Pair.Key.Get())
		{
			PDPlayerState->OnCharacterCustomInfoChangedNative().Remove(Pair.Value);
		}
	}

	CharacterCustomInfoChangedHandles.Reset();
}

void UServerLobby::HandleLobbyPlayerCharacterCustomInfoChanged(const FPDCharacterCustomInfo& NewCharacterCustomInfo)
{
	RefreshLobbyTeamInfos();
}

void UServerLobby::StartLobbyTeamRefreshRetry()
{
	if (!GetWorld())
	{
		return;
	}

	if (LobbyTeamRefreshRetryCount >= 20)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[LobbyTeamRefreshRetry] Max retries reached. CachedTeamInfoCount=%d"),
			CachedTeamInfos.Num());
		StopLobbyTeamRefreshRetry();
		return;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(LobbyTeamRefreshRetryHandle))
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[LobbyTeamRefreshRetry] Scheduling retry Attempt=%d"),
		LobbyTeamRefreshRetryCount + 1);

	GetWorld()->GetTimerManager().SetTimer(
		LobbyTeamRefreshRetryHandle,
		this,
		&UServerLobby::RetryRefreshLobbyTeamInfos,
		0.2f,
		false);
}

void UServerLobby::StopLobbyTeamRefreshRetry()
{
	LobbyTeamRefreshRetryCount = 0;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(LobbyTeamRefreshRetryHandle);
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
	UnbindCharacterCustomInfoDelegates();
	StopLobbyTeamRefreshRetry();
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
