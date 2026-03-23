// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/Shop/FPDItemInfo.h"
#include "IngameHUD.generated.h"

class UPD_InventoryUI;
class UPD_ShopUI;
/**
 * 
 */
UCLASS()
class PROJECTD_API UIngameHUD : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void ToggleGameUI();

	UFUNCTION()
	void InitGold(int NewGold);

	UFUNCTION()
	void InitItem(EItemType ItemType, int SlotIndex, const FName& NewItemID, int Count = 0);

	UFUNCTION()
	void InitUI();

protected:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPD_InventoryUI> InventoryUI;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPD_ShopUI> ShopUI;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> OpenShopUI;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> CloseShopUI;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> OpenInventoryUI;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> CloseInventoryUI;

	UFUNCTION()
	void OnUIOpenFinished();

	UFUNCTION()
	void OnUICloseFinished();

	int32 OpenedUIPriority = 0;
	bool bIsUIPanelOpen = false;
};