// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "BlueprintDataDefinitions.h"
#include "GameMode/PDLobbyGameMode.h"
#include "TimerManager.h"
#include "ServerLobby.generated.h"

class APDPlayerState;
class UTextBlock;
class UBorder;

UENUM(BlueprintType)
enum class ELobbyAvatarTarget : uint8
{
	LocalTeam UMETA(DisplayName = "LocalTeam"),
	OtherTeamB UMETA(DisplayName = "OtherTeamB"),
	OtherTeamC UMETA(DisplayName = "OtherTeamC")
};

/**
 * 
 */
UCLASS()
class PROJECTD_API UServerLobby : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	void ApplyLobbyTeamInfos(const TArray<FTeamInfo>& InTeamInfos, ETeamType InLocalTeamID);
	void ApplyMatchStartServerTime(float InMatchStartServerTimeSec);

protected:
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateLobbyMemberAvatar(ELobbyAvatarTarget AvatarTarget, int32 SlotIndex, bool bHasPlayer, const FBPUniqueNetId& UniqueNetId);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamAInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamBInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamCInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MatchingTime;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> TeamPanel_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> TeamPanel_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamNickName_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamNickName_1;
	
private:
	void UpdateTeamInfoText(
		UTextBlock* TextBlock,
		const TCHAR* TeamLabel,
		const FTeamInfo* TeamInfo,
		ETeamType TeamID,
		ETeamType LocalTeamID);
	void RefreshLobbyTeamInfos();
	void UpdateLocalTeamPanels(ETeamType LocalTeamID);
	void CollectLocalTeamPlayerStates(ETeamType LocalTeamID, TArray<const APDPlayerState*>& OutTeamMembers) const;
	void CollectTeamPlayerStates(ETeamType TeamID, TArray<const APDPlayerState*>& OutTeamMembers) const;
	void UpdateOtherTeamAvatars(ETeamType LocalTeamID);
	void NotifyAvatarTarget(ELobbyAvatarTarget AvatarTarget, int32 SlotIndex, const APDPlayerState* SlotPlayerState, ETeamType SourceTeamID);
	int32 GetExpectedReadyPlayerCount(ETeamType TeamID) const;
	int32 CountReadyPlayerStates(ETeamType TeamID) const;
	bool AreAllExpectedPlayerStatesReady() const;
	void RetryRefreshLobbyTeamInfos();
	void StartLobbyTeamRefreshRetry();
	void StopLobbyTeamRefreshRetry();

	void RefreshMatchingTimeText();
	void StartMatchingTimeRefresh();
	void StopMatchingTimeRefresh();

	TArray<FTeamInfo> CachedTeamInfos;
	ETeamType CachedLocalTeamID = ETeamType::None;
	FTimerHandle LobbyTeamRefreshRetryHandle;
	int32 LobbyTeamRefreshRetryCount = 0;

	FTimerHandle MatchingTimeRefreshHandle;
	float MatchStartServerTimeSec = 0.0f;
	bool bHasMatchStartServerTime = false;
};
