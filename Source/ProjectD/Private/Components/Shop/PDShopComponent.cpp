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
            UE_LOG(LogTemp, Warning, TEXT("ServerCall"));
            AcceptanceBuy(ItemId);
        }
    }
}

void UPDShopComponent::AcceptanceBuy_Implementation(FName ItemId)
{
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("UPDShopComponent::AcceptanceBuy_Implementation GI Null"));
        return;
    }

    UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
    if (!ItemSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UPDShopComponent::AcceptanceBuy_Implementation ItemSubsystem Null"));
        return;
    }

    const FPDItemInfo* ItemData = ItemSubsystem->GetItemInfoByName(ItemId);
    if (!ItemData)
    {
        UE_LOG(LogTemp, Error, TEXT("UPDShopComponent::AcceptanceBuy_Implementation ItemData Null"));
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("UPDShopComponent::AcceptanceBuy_Implementation PC Null"));
        return;
    }

    APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Error, TEXT("UPDShopComponent::AcceptanceBuy_Implementation PS Null"));
        return;
    }

    UPDInventoryComponent* Inventory = PS->GetInventoryComponent();

    if (!Inventory)
    {
        UE_LOG(LogTemp, Error, TEXT("UPDShopComponent::AcceptanceBuy_Implementation Inventory Null"));
        return;
    }

    const int32 TotalCost = ItemData->Price;

    if (!Inventory->CheckGold(TotalCost))
    {
        UE_LOG(LogTemp, Error, TEXT("UPDShopComponent::AcceptanceBuy_Implementation CheckGold Fail"));
        return;
    }

    if (!Inventory->AddItem(ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("UPDShopComponent::AcceptanceBuy_Implementation AddItem Fail"));
        return;
    }

    Inventory->AddGold(-TotalCost);
}



