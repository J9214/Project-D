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
class UWidget;

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
	void BP_UpdateTeamMemberAvatar(int32 SlotIndex, const FBPUniqueNetId& UniqueNetId);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamAInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamBInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamCInfo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MatchingTime;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> TeamPanel_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> TeamPanel_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamNickName_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamNickName_1;
	
private:
	void UpdateTeamInfoText(
		UTextBlock* TextBlock,
		const TCHAR* TeamLabel,
		const FTeamInfo* TeamInfo,
		bool bIsMyTeam);
	void UpdateLocalTeamPanels(ETeamType LocalTeamID);
	void CollectLocalTeamPlayerStates(ETeamType LocalTeamID, TArray<const APDPlayerState*>& OutTeamMembers) const;

	void RefreshMatchingTimeText();
	void StartMatchingTimeRefresh();
	void StopMatchingTimeRefresh();

	FTimerHandle MatchingTimeRefreshHandle;
	float MatchStartServerTimeSec = 0.0f;
	bool bHasMatchStartServerTime = false;
};
