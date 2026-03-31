// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/IngameHUD.h"
#include "Animation/WidgetAnimation.h"
#include "UI/Inventory/PD_InventoryUI.h"
#include "UI/Shop/PD_ShopUI.h"
#include "UI/Ingame/PDIngameInfo.h"
#include "UI/Ingame/PDTeamHPInfo.h"
#include "Components/TextBlock.h"
#include "PlayerState/PDPlayerState.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "UI/PDTeamColorFunctionLibrary.h"

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

    HPBars.Add(EHPBarSlot::Player, PlayerHPBar);
    HPBars.Add(EHPBarSlot::Team1, TeamHPBar);

    RefreshTeamScoreColors();
}

void UIngameHUD::NativeConstruct()
{
    Super::NativeConstruct();

    bIsFocusable = true;
    RefreshTeamScoreColors();
}

void UIngameHUD::RefreshTeamScoreColors()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    const APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>();
    if (!PS)
    {
        return;
    }

    const ETeamType LocalTeamID = PS->GetTeamID();

    if (Team1Score)
    {
        Team1Score->SetColorAndOpacity(UPDTeamColorFunctionLibrary::GetRelativeTeamSlateColorByIndex(LocalTeamID, 0));
    }

    if (Team2Score)
    {
        Team2Score->SetColorAndOpacity(UPDTeamColorFunctionLibrary::GetRelativeTeamSlateColorByIndex(LocalTeamID, 1));
    }

    if (Team3Score)
    {
        Team3Score->SetColorAndOpacity(UPDTeamColorFunctionLibrary::GetRelativeTeamSlateColorByIndex(LocalTeamID, 2));
    }
}

void UIngameHUD::BindSlot(const FString& DisplayName, EHPBarSlot InSlot, UPDAttributeSetBase* Set, ETeamType LocalTeamID, ETeamType TargetTeamID)
{
    if (UPDTeamHPInfo* Bar = HPBars.FindRef(InSlot))
    {
        if (Bar->CheckInit())
        {
            return;
        }

        Bar->Init(DisplayName);
        switch (InSlot)
        {
        case EHPBarSlot::Player:
        {
            Bar->SetPlayerColor();
            Bar->SetTeamTextColor(LocalTeamID, TargetTeamID);
            break;
        }
        case EHPBarSlot::Team1:
        {
            Bar->SetTeamColor(0);
            Bar->SetTeamTextColor(LocalTeamID, TargetTeamID);
            break;
        }
        default: break;
        }
    }

    BoundAttrSets.FindOrAdd(InSlot) = Set;

    UPDAttributeSetBindProxy* Proxy = BindProxies.FindRef(InSlot);
    if (!IsValid(Proxy))
    {
        Proxy = NewObject<UPDAttributeSetBindProxy>(this);
        BindProxies.Add(InSlot, Proxy);
    }

    Proxy->Init(this, InSlot, Set);
}

void UIngameHUD::HandleHealthChangedBySlot(EHPBarSlot InSlot, float OldValue, float NewValue)
{
    if (UPDTeamHPInfo* Bar = HPBars.FindRef(InSlot))
    {
        float Value = Bar->HandleHealthChanged(OldValue, NewValue);
    }
}

void UIngameHUD::ToggleGameUI()
{
    if (IsAnimationPlaying(OpenShopUI) || IsAnimationPlaying(CloseShopUI) ||
        IsAnimationPlaying(OpenInventoryUI) || IsAnimationPlaying(CloseInventoryUI))
    {
        return;
    }

    bool bIsDead = false; 

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        if (APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>())
        {
            if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
            {
                bIsDead = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
            }
        }
    }

    if (!bIsUIPanelOpen)
    {
        if (bIsDead)
        {
            PlayAnimation(OpenShopUI);
            PlayAnimation(OpenInventoryUI);
            bIsShopOpen = true;
        }
        else
        {
            PlayAnimation(OpenInventoryUI);
            bIsShopOpen = false;
        }
        bIsUIPanelOpen = true;
    }
    else
    {
        if (bIsShopOpen)
        {
            PlayAnimation(CloseShopUI);
            bIsShopOpen = false;
        }

        PlayAnimation(CloseInventoryUI);
        bIsUIPanelOpen = false;
    }
}

void UIngameHUD::ForceOpenDeadShopUI()
{
    if (CloseShopUI)
    {
        StopAnimation(CloseShopUI);
    }

    if (CloseInventoryUI)
    {
        StopAnimation(CloseInventoryUI);
    }

    if (!bIsUIPanelOpen)
    {
        OpenedUIPriority = 0;
        bIsShopOpen = false;

        if (OpenInventoryUI)
        {
            PlayAnimation(OpenInventoryUI);
        }
        else if (OpenedUIPriority <= 0)
        {
            OnUIOpenFinished();
        }

        bIsUIPanelOpen = true;
    }
    else if (OpenedUIPriority <= 0)
    {
        OnUIOpenFinished();
    }

    if (!bIsShopOpen)
    {
        if (OpenShopUI)
        {
            PlayAnimation(OpenShopUI);
        }

        bIsShopOpen = true;
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

void UIngameHUD::UpdateCurrentAmmo(int32 CurrentAmmo)
{
    PlayerIngameInfo->UpdateCurrentAmmo(CurrentAmmo);
}

void UIngameHUD::OnShopUI()
{
    ForceOpenDeadShopUI();
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
