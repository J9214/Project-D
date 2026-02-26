// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PD_InventoryUI.generated.h"

class UPD_ItemSlot;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTD_API UPD_InventoryUI : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeOnInitialized() override;

	void InitWeaponSlot(int SlotIndex, const FName& NewItemID);
	void InitSkillSlot(int SlotIndex, const FName& NewItemID);
	void InitThrowSlot(int SlotIndex, const FName& NewItemID, int32 NewCount);
	void InitItemSlot(int SlotIndex, const FName& NewItemID, int32 NewCount);

	void InitGold(int NewGold);
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> WeaponSlot1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> WeaponSlot2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> SkillSlot1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> SkillSlot2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ThrowSlot1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ThrowSlot2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ItemSlot1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ItemSlot2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ItemSlot3;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ItemSlot4;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ItemSlot5;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ItemSlot6;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ItemSlot7;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPD_ItemSlot> ItemSlot8;


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Gold;

	int GoldValue = 0;
};
