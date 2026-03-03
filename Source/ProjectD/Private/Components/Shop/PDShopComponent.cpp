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
        UE_LOG(LogTemp, Warning, TEXT("1"));
        return;
    }

    UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
    if (!ItemSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("2"));
        return;
    }

    const FPDItemInfo* ItemData = ItemSubsystem->GetItemInfoByName(ItemId);
    if (!ItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("3"));
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("4"));
        return;
    }

    APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("5"));
        return;
    }

    UPDInventoryComponent* Inventory = PS->GetInventoryComponent();

    if (!Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("6"));
        return;
    }

    const int32 TotalCost = ItemData->Price;

    if (!Inventory->CheckGold(TotalCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("7"));
        return;
    }

    if (!Inventory->AddItem(ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("8"));
        return;
    }

    Inventory->AddGold(-TotalCost);
}



