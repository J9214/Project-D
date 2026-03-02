// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PD_ShopUI.h"
#include "Components/Button.h"
#include "CommonTileView.h"
#include "GameInstance/Subsystem/PDItemInfoSubsystem.h"
#include "Components/Shop/ItemDataWrapper.h"
#include "Components/TextBlock.h"

void UPD_ShopUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
    if (WeaponTab)
    {
        WeaponTab->OnClicked.AddDynamic(this, &ThisClass::HandleWeaponTabClicked);
    }
    if (SkillTab)
    {
        SkillTab->OnClicked.AddDynamic(this, &ThisClass::HandleSkillTabClicked);
    }
    if (ItemTab)
    {
        ItemTab->OnClicked.AddDynamic(this, &ThisClass::HandleItemTabClicked);
    }

	SetInit();
}

void UPD_ShopUI::SetInit()
{
	if (bIsInitialized)
	{
		return;
	}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		return;
	}

	UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
	if (!ItemSubsystem)
	{
		return;
	}

	const UDataTable* DT = ItemSubsystem->GetDataTable();
	if (!DT)
	{
		return;
	}

    TArray<FPDItemInfo*> AllRows;
	DT->GetAllRows<FPDItemInfo>(TEXT("ShopInit"), AllRows);

    for (const FPDItemInfo* Info : AllRows)
    {
		if (!Info)
		{
			continue;
		}

        UItemDataWrapper* NewItem = NewObject<UItemDataWrapper>(this);
        NewItem->ItemID = Info->ItemID;

		if (Info->ItemType == EItemType::Weapon)
		{
			WeaponTileView->AddItem(NewItem);
		}
		else if (Info->ItemType == EItemType::Skill)
		{
			SkillTileView->AddItem(NewItem);
		}
		else if (Info->ItemType == EItemType::Grenade || Info->ItemType == EItemType::Etc)
		{
			ItemTileView->AddItem(NewItem);
		}
    }

    bIsInitialized = true;

    HandleWeaponTabClicked();
}

void UPD_ShopUI::HandleWeaponTabClicked()
{
	SetInit();
	ResetTabButtons();
	if (WeaponTab)
	{
		WeaponTab->SetIsEnabled(false);
	}

	if (WeaponTileView)
	{
		WeaponTileView->SetVisibility(ESlateVisibility::Visible);
	}
	if (SkillTileView)
	{
		SkillTileView->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ItemTileView)
	{
		ItemTileView->SetVisibility(ESlateVisibility::Collapsed);
	}

}

void UPD_ShopUI::HandleSkillTabClicked()
{
	SetInit();
	ResetTabButtons();
	if (SkillTab)
	{
		SkillTab->SetIsEnabled(false);
	}

	if (WeaponTileView)
	{
		WeaponTileView->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (SkillTileView)
	{
		SkillTileView->SetVisibility(ESlateVisibility::Visible);
	}
	if (ItemTileView)
	{
		ItemTileView->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPD_ShopUI::HandleItemTabClicked()
{
	SetInit();
	ResetTabButtons();
	if (ItemTab)
	{
		ItemTab->SetIsEnabled(false);
	}

	if (WeaponTileView)
	{
		WeaponTileView->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (SkillTileView)
	{
		SkillTileView->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ItemTileView)
	{
		ItemTileView->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPD_ShopUI::ResetTabButtons()
{
	if (WeaponTab)
	{
		WeaponTab->SetIsEnabled(true);
	}
	if (SkillTab)
	{
		SkillTab->SetIsEnabled(true);
	}
	if (ItemTab)
	{
		ItemTab->SetIsEnabled(true);
	}
}

void UPD_ShopUI::InitGold(int NewGold)
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