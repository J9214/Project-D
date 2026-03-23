// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PD_DragVisual.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UPD_DragVisual::SetActionText(const FString& NewText, bool bVisible)
{
    if (ActionText)
    {
        ActionText->SetText(FText::FromString(NewText));
        ActionText->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
}

void UPD_DragVisual::SetDragVisual(const FSlateBrush& InBrush, int32 InCount)
{
    if (DragIcon)
    {
        DragIcon->SetBrush(InBrush);
    }

    if (Count)
    {
        if (InCount > 1)
        {
            Count->SetText(FText::AsNumber(InCount));
            Count->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        else
        {
            Count->SetText(FText::GetEmpty());
            Count->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}
