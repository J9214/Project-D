// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PD_ItemPurchaseButton.h"
#include <Components/Shop/FPDItemInfo.h>
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include <Controller/PDPlayerController.h>
#include "Components/Shop/PDShopComponent.h"
#include <PlayerState/PDPlayerState.h>
#include "Components/Inventory/PDInventoryComponent.h"
#include <Components/Shop/ItemDataWrapper.h>
#include <GameInstance/Subsystem/PDItemInfoSubsystem.h>

void UPD_ItemPurchaseButton::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    auto* Entry = Cast<UItemDataWrapper>(ListItemObject);
    if (!Entry)
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

    const FPDItemInfo* Info = ItemSubsystem->GetItemInfoByName(Entry->ItemID);
    if (!Info)
    {
        return;
    }

    ItemID = Entry->ItemID;

    if (IconImage)
    {
        UTexture2D* Tex = nullptr;

        if (Info->IconImage.IsValid())
            Tex = Info->IconImage.Get();
        else
            Tex = Info->IconImage.LoadSynchronous();

        IconImage->SetBrushFromTexture(Tex, true);
    }

    if (ItemDisplayName)
    {
        ItemDisplayName->SetText(FText::FromName(Info->DisplayName));
    }

    if (Price)
    {
        Price->SetText(FText::AsNumber(Info->Price));
    }

    APlayerController* PC = GetOwningPlayer();
    if (!IsValid(PC))
    {
        return;
    }

    APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>();
    if (!PS)
    {
        return;
    }

    if (UPDInventoryComponent* Inventory = PS->GetInventoryComponent())
    {
        bool bCanAfford = Inventory->CheckGold(Info->Price);
        this->SetIsEnabled(bCanAfford);
    }

}

void UPD_ItemPurchaseButton::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    this->OnClicked().AddUObject(this, &ThisClass::HandleItemPurchase);
}

void UPD_ItemPurchaseButton::HandleItemPurchase()
{
    UE_LOG(LogTemp, Warning, TEXT("BuyTry1"));
    APDPlayerController* PC = GetOwningPlayer<APDPlayerController>();
    if (!IsValid(PC))
    {
        return;
    }

    UPDShopComponent* ShopComponent = PC->GetShopComponent();
    if (!ShopComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("BuyTry"));
    ShopComponent->RequestBuy(ItemID);
}
