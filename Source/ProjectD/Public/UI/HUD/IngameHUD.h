// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/Shop/FPDItemInfo.h"
#include "UI/Ingame/PDAttributeSetBindProxy.h"
#include "IngameHUD.generated.h"

class UPD_InventoryUI;
class UPD_ShopUI;
class UPDIngameInfo;
class UPDTeamHPInfo;
class UPDAttributeSetBase;
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

	void BindSlot(const FString& DisplayName, EHPBarSlot InSlot, UPDAttributeSetBase* Set);

	UFUNCTION()
	void HandleHealthChangedBySlot(EHPBarSlot InSlot, float OldValue, float NewValue);

	UFUNCTION()
	void ToggleGameUI();

	UFUNCTION()
	void ForceOpenDeadShopUI();

	UFUNCTION()
	void InitGold(int NewGold);

	UFUNCTION()
	void InitItem(EItemType ItemType, int SlotIndex, const FName& NewItemID, int Count = 0);

	UFUNCTION()
	void UpdateCurrentAmmo(int32 CurrentAmmo);

	UFUNCTION(Exec)
	void OnShopUI();

protected:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPD_InventoryUI> InventoryUI;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPD_ShopUI> ShopUI;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPDIngameInfo> PlayerIngameInfo;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPDTeamHPInfo> PlayerHPBar;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPDTeamHPInfo> TeamHPBar;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> OpenShopUI;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> CloseShopUI;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> OpenInventoryUI;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> CloseInventoryUI;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UPDAttributeSetBase>> BoundAttrSets;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UPDAttributeSetBindProxy>> BindProxies;

	UPROPERTY()
	TMap<EHPBarSlot, TObjectPtr<UPDTeamHPInfo>> HPBars;

	UFUNCTION()
	void OnUIOpenFinished();

	UFUNCTION()
	void OnUICloseFinished();

	int32 OpenedUIPriority = 0;
	bool bIsUIPanelOpen = false;
	bool bIsShopOpen = false;
};
