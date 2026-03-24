// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/IngameHUD.h"
#include "Animation/WidgetAnimation.h"
#include "UI/Inventory/PD_InventoryUI.h"
#include "UI/Shop/PD_ShopUI.h"

void UIngameHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    FWidgetAnimationDynamicEvent OpenDelegate;
    OpenDelegate.BindDynamic(this, &ThisClass::OnUIOpenFinished);

    if (OpenInventoryUI)
    {
        BindToAnimationFinished(OpenInventoryUI, OpenDelegate);
    }

    FWidgetAnimationDynamicEvent CloseDelegate;
    CloseDelegate.BindDynamic(this, &ThisClass::OnUICloseFinished);

    if (CloseInventoryUI)
    {
        BindToAnimationFinished(CloseInventoryUI, CloseDelegate);
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = false;
}

void UIngameHUD::NativeConstruct()
{
    Super::NativeConstruct();

    bIsFocusable = true;
}

void UIngameHUD::ToggleGameUI()
{
    if (IsAnimationPlaying(OpenShopUI) || IsAnimationPlaying(CloseShopUI) ||
        IsAnimationPlaying(OpenInventoryUI) || IsAnimationPlaying(CloseInventoryUI))
    {
        return;
    }

    if (!bIsUIPanelOpen)
    {
        PlayAnimation(OpenShopUI);
        PlayAnimation(OpenInventoryUI);
        bIsUIPanelOpen = true;
    }
    else
    {
        PlayAnimation(CloseShopUI);
        PlayAnimation(CloseInventoryUI);
        bIsUIPanelOpen = false;
    }
}

void UIngameHUD::InitItem(EItemType ItemType, int SlotIndex, const FName& NewItemID, int Count)
{
    if (!InventoryUI)
    {
        return;
    }

    switch (ItemType)
    {
    case EItemType::Weapon:
        InventoryUI->InitWeaponSlot(SlotIndex, NewItemID);
        break;
    case EItemType::Skill:
        InventoryUI->InitSkillSlot(SlotIndex, NewItemID);
        break;
    case EItemType::Grenade:
        InventoryUI->InitThrowSlot(SlotIndex, NewItemID, Count);
        break;
    case EItemType::Etc:
        InventoryUI->InitItemSlot(SlotIndex, NewItemID, Count);
        break;
    default:
        break;
    }
}

void UIngameHUD::InitUI()
{


}


void UIngameHUD::InitGold(int NewGold)
{
    if (InventoryUI)
    {
        InventoryUI->InitGold(NewGold);
    }

    if (ShopUI)
    {
        ShopUI->InitGold(NewGold);
    }
}


void UIngameHUD::OnUIOpenFinished()
{
    OpenedUIPriority++;

    if (OpenedUIPriority > 1)
    {
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetWidgetToFocus(TakeWidget()); 
    InputMode.SetWidgetToFocus(GetCachedWidget());
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = true;
    SetKeyboardFocus();
}

void UIngameHUD::OnUICloseFinished()
{
    OpenedUIPriority = FMath::Max(0, OpenedUIPriority - 1);

    if (OpenedUIPriority > 0)
    {
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = false; 
}