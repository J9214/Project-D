// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PD_InventoryUI.h"
#include "UI/Inventory/PD_ItemSlot.h"
#include "Components/TextBlock.h"
#include <Components/Inventory/PDInventoryComponent.h>

void UPD_InventoryUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPD_InventoryUI::InitWeaponSlot(int SlotIndex, const FName& NewItemID)
{
	UPD_ItemSlot* TargetSlot = nullptr;

	if (SlotIndex == 0)
	{
		TargetSlot = WeaponSlot1;
	}
	else if (SlotIndex == 1)
	{
		TargetSlot = WeaponSlot2;
	}

	if (TargetSlot)
	{
		TargetSlot->UpdateSlot(NewItemID, 0);
	}
}

void UPD_InventoryUI::InitSkillSlot(int SlotIndex, const FName& NewItemID)
{
	UPD_ItemSlot* TargetSlot = nullptr;

	if (SlotIndex == 0)
	{
		TargetSlot = SkillSlot1;
	}
	else if (SlotIndex == 1)
	{
		TargetSlot = SkillSlot2;
	}

	if (TargetSlot)
	{
		TargetSlot->UpdateSlot(NewItemID, 0);
	}
}

void UPD_InventoryUI::InitThrowSlot(int SlotIndex, const FName& NewItemID, int32 NewCount)
{
	UPD_ItemSlot* TargetSlot = nullptr;

	if (SlotIndex == 0)
	{
		TargetSlot = ThrowSlot1;
	}
	else if (SlotIndex == 1)
	{
		TargetSlot = ThrowSlot2;
	}

	if (TargetSlot)
	{
		TargetSlot->UpdateSlot(NewItemID, NewCount);
	}
}

void UPD_InventoryUI::InitItemSlot(int SlotIndex, const FName& NewItemID, int32 NewCount)
{
	UPD_ItemSlot* TargetSlot = nullptr;

	switch (SlotIndex)
	{
	case 0: TargetSlot = ItemSlot1; break;
	case 1: TargetSlot = ItemSlot2; break;
	case 2: TargetSlot = ItemSlot3; break;
	case 3: TargetSlot = ItemSlot4; break;
	case 4: TargetSlot = ItemSlot5; break;
	case 5: TargetSlot = ItemSlot6; break;
	case 6: TargetSlot = ItemSlot7; break;
	case 7: TargetSlot = ItemSlot8; break;
	default: break;
	}

	if (TargetSlot)
	{
		TargetSlot->UpdateSlot(NewItemID, NewCount);
	}
}

void UPD_InventoryUI::InitGold(int NewGold)
{
	if (NewGold == GoldValue)
	{
		return;
	}

	if (Gold)
	{
		GoldValue = NewGold;
		Gold->SetText(FText::AsNumber(GoldValue));
	}
}
