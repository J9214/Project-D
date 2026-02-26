// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Shop/PDShopComponent.h"
#include "Components/Inventory/PDInventoryComponent.h"
#include "PlayerState/PDPlayerState.h"
#include <GameInstance/Subsystem/PDItemInfoSubsystem.h>

UPDShopComponent::UPDShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UPDShopComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPDShopComponent::RequestBuy(FName ItemId)
{
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
    {
        if (PC->IsLocalController())
        {
            AcceptanceBuy(ItemId);
        }
    }
}

void UPDShopComponent::AcceptanceBuy_Implementation(FName ItemId)
{
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

    const FPDItemInfo* ItemData = ItemSubsystem->GetItemInfoByName(ItemId);

    if (!ItemData)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        return;
    }

    APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>();
    if (!PS)
    {
        return;
    }

    UPDInventoryComponent* Inventory = PS->GetInventoryComponent();

    if (!Inventory)
    {
        return;
    }

    const int32 TotalCost = ItemData->Price;

    if (!Inventory->CheckGold(TotalCost))
    {
        return;
    }

    if (!Inventory->AddItem(ItemData))
    {
        return;
    }

    Inventory->AddGold(-TotalCost);
}



