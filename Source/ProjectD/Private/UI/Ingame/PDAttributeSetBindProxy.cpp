// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/PDAttributeSetBindProxy.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "UI/HUD/IngameHUD.h"

void UPDAttributeSetBindProxy::Init(UIngameHUD* InHUD, EHPBarSlot InSlot, UPDAttributeSetBase* InSet)
{
    HUD = InHUD;
    Slot = InSlot;

    if (BindSet == InSet)
    {
        return;
    }

    Unbind();

    BindSet = InSet;
    if (!IsValid(BindSet) || !IsValid(HUD))
    {
        return;
    }

    BindSet->OnHealthChanged.AddDynamic(this, &ThisClass::OnHealth);

    OnHealth(BindSet->GetHealth(), BindSet->GetHealth());
}

void UPDAttributeSetBindProxy::Unbind()
{
    if (!IsValid(BindSet))
    {
        return;
    }

    BindSet->OnHealthChanged.RemoveDynamic(this, &ThisClass::OnHealth);
    BindSet = nullptr;
}


void UPDAttributeSetBindProxy::OnHealth(float OldValue, float NewValue)
{
    if (IsValid(HUD))
    {
        HUD->HandleHealthChangedBySlot(Slot, OldValue, NewValue);
    }
}



