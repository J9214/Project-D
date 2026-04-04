// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "FriendsLobby.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class PROJECTD_API UFriendsLobby : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void BP_InitTeamInfoDisplay();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitFriendsPopupButton(bool Active);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClosePopupAnimation();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FriendsPopupButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GameStart;
protected:

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OpenFriendsList;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> CloseFriendsList;

	bool IsActiveFriendsPopup = false;

	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleFriendsPopupClicked();
};
