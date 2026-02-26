// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PD_ItemSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameInstance/Subsystem/PDItemInfoSubsystem.h"
#include "Components/Shop/FPDItemInfo.h"

void UPD_ItemSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UpdateSlot(NAME_None, 0);
}

void UPD_ItemSlot::UpdateSlot(const FName& NewItemID, int32 NewCount)
{
    ItemID = NewItemID;

    if (ItemID.IsNone())
    {
        if (IconImage && EmptyIconTexture)
        {
            IconImage->SetBrushFromTexture(EmptyIconTexture, true);
        }

        if (ItemDisplayName)
        {
            ItemDisplayName->SetText(FText::GetEmpty());
            ItemDisplayName->SetVisibility(ESlateVisibility::Hidden);
        }

        if (Count)
        {
            Count->SetVisibility(ESlateVisibility::Hidden);
        }

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

    const FPDItemInfo* Info = ItemSubsystem->GetItemInfoByName(ItemID);
    if (!Info)
    {
        return;
    }

    UTexture2D* Tex = Info->IconImage.LoadSynchronous();
    if (IconImage)
    {
        IconImage->SetBrushFromTexture(Tex, true);
    }

    if (ItemDisplayName)
    {
        ItemDisplayName->SetText(FText::FromName(Info->DisplayName));
    }

    if (Count && NewCount != 0)
    {
        Count->SetText(FText::AsNumber(NewCount));
        Count->SetVisibility(NewCount > 1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}
