// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/Shop/FPDItemInfo.h"
#include "PD_ShopUI.generated.h"

class UCommonTileView;
class UPD_ItemPurchaseButton;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTD_API UPD_ShopUI : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	
	void SetInit();
	void InitGold(int NewGold);
protected:


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WeaponTab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillTab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ItemTab;


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTileView> WeaponTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTileView> SkillTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTileView> ItemTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Gold;

	int GoldValue = 0;

	UFUNCTION()
	void HandleWeaponTabClicked();

	UFUNCTION()
	void HandleSkillTabClicked();

	UFUNCTION()
	void HandleItemTabClicked();

	UFUNCTION()
	void ResetTabButtons();

	UPROPERTY()
	bool bIsInitialized;

};
