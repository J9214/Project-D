// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/PDAttributeSetBindProxy.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "UI/HUD/IngameHUD.h"

void UPDAttributeSetBindProxy::Init(UIngameHUD* InHUD, EHPBarSlot InSlot, UPDAttributeSetBase* InSet)
{
    HUD = InHUD;
    Slot = InSlot;

    if (BoundSet == InSet)
    {
        return;
    }

    Unbind();

    BoundSet = InSet;
    if (!IsValid(BoundSet) || !IsValid(HUD))
    {
        return;
    }

    BoundSet->OnHealthChanged.AddDynamic(this, &ThisClass::OnHealth);

    OnHealth(BoundSet->GetHealth(), BoundSet->GetHealth());
}

void UPDAttributeSetBindProxy::Unbind()
{
    if (!IsValid(BoundSet))
    {
        return;
    }

    BoundSet->OnHealthChanged.RemoveDynamic(this, &ThisClass::OnHealth);
    BoundSet = nullptr;
}


void UPDAttributeSetBindProxy::OnHealth(float OldValue, float NewValue)
{
    if (IsValid(HUD))
    {
        HUD->HandleHealthChangedBySlot(Slot, OldValue, NewValue);
    }
}



