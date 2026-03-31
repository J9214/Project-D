// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/PDAttributeSetBindProxy.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "UI/HUD/IngameHUD.h"

void UPDAttributeSetBindProxy::Init(UIngameHUD* InHUD, EHPBarSlot InSlot, UPDAttributeSetBase* InSet)
{
    HUD = InHUD;
    Slot = InSlot;

    if (BindSet != InSet)
    {
        Unbind();
        BindSet = InSet;

        if (IsValid(BindSet))
        {
            BindSet->OnHealthChanged.AddUniqueDynamic(this, &ThisClass::OnHealth);
            BindSet->OnMaxHealthChanged.AddUniqueDynamic(this, &ThisClass::OnMaxHealth);
        }
    }

    if (!IsValid(BindSet) || !IsValid(HUD))
    {
        return;
    }

    OnMaxHealth(BindSet->GetMaxHealth(), BindSet->GetMaxHealth());
    OnHealth(BindSet->GetHealth(), BindSet->GetHealth());
}

void UPDAttributeSetBindProxy::Unbind()
{
    if (!IsValid(BindSet))
    {
        return;
    }

    BindSet->OnHealthChanged.RemoveDynamic(this, &ThisClass::OnHealth);
    BindSet->OnMaxHealthChanged.RemoveDynamic(this, &ThisClass::OnMaxHealth);
    BindSet = nullptr;
}


void UPDAttributeSetBindProxy::OnHealth(float OldValue, float NewValue)
{
    if (IsValid(HUD))
    {
        HUD->HandleHealthChangedBySlot(Slot, OldValue, NewValue);
    }
}

void UPDAttributeSetBindProxy::OnMaxHealth(float OldValue, float NewValue)
{
    if (IsValid(HUD))
    {
        HUD->HandleMaxHealthChangedBySlot(Slot, OldValue, NewValue);
    }
}



