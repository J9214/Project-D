// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PD_ItemSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameInstance/Subsystem/PDItemInfoSubsystem.h"
#include "Components/Shop/FPDItemInfo.h"
#include <UI/Inventory/PD_ItemDragDropOperation.h>
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "UI/Inventory/PD_DragVisual.h"
#include <UI/HUD/IngameHUD.h>
#include <Components/Inventory/PDInventoryComponent.h>
#include <PlayerState/PDPlayerState.h>

void UPD_ItemSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UpdateSlot(NAME_None, 0);
}

void UPD_ItemSlot::UpdateSlot(const FName& NewItemID, int32 NewCount)
{
    ItemID = NewItemID;

    if (ItemID.IsNone())
    {
        if (IconImage && EmptyIconTexture)
        {
            IconImage->SetBrushFromTexture(EmptyIconTexture, true);
        }

        if (ItemDisplayName)
        {
            ItemDisplayName->SetText(FText::GetEmpty());
            ItemDisplayName->SetVisibility(ESlateVisibility::Hidden);
        }

        Count = NewCount;

        if (CountText && NewCount == 0)
        {
            CountText->SetVisibility(ESlateVisibility::Hidden);
        }

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

    const FPDItemInfo* Info = ItemSubsystem->GetItemInfoByName(ItemID);
    if (!Info)
    {
        return;
    }

    UTexture2D* Tex = Info->IconImage.LoadSynchronous();
    if (IconImage)
    {
        IconImage->SetBrushFromTexture(Tex, true);
    }

    if (ItemDisplayName)
    {
        ItemDisplayName->SetText(FText::FromName(Info->DisplayName));
    }

    if (CountText && NewCount != 0)
    {
        Count = NewCount;
        CountText->SetText(FText::AsNumber(NewCount));
        CountText->SetVisibility(NewCount > 1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

FReply UPD_ItemSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

    if (ItemID.IsNone())
    {
        return FReply::Unhandled();
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }

    return FReply::Unhandled();
}

void UPD_ItemSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (ItemID.IsNone())
    {
        return;
    }

    UPD_ItemDragDropOperation* DragOp = NewObject<UPD_ItemDragDropOperation>();
    if (!DragOp)
    {
        return;
    }

    DragOp->ItemID = ItemID;
    DragOp->SourceSlot = this;
    DragOp->ItemType = ItemType;
    DragOp->Pivot = EDragPivot::MouseDown;

    if (DragVisualClass)
    {
        UPD_DragVisual* VisualWidget = CreateWidget<UPD_DragVisual>(GetWorld(), DragVisualClass);
        if (VisualWidget)
        {
            if (IconImage)
            {
                VisualWidget->SetDragVisual(IconImage->GetBrush(), Count);
            }

            VisualWidget->SetActionText(TEXT(""), false);

            DragOp->DefaultDragVisual = VisualWidget;
            DragOp->DragVisualWidget = VisualWidget;
        }
    }

    OutOperation = DragOp;

}

void UPD_ItemSlot::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

    if (UPD_ItemDragDropOperation* ItemOp = Cast<UPD_ItemDragDropOperation>(InOperation))
    {
        if (ItemOp->SourceSlot != this && ItemOp->ItemType == this->ItemType)
        {
            if (ItemOp->DragVisualWidget)
            {
                FString ActionMsg = ItemID.IsNone() ? TEXT("장착하기") : TEXT("교체하기");
                ItemOp->DragVisualWidget->SetActionText(ActionMsg, true);
            }
        }
        else
        {
            if (ItemOp->DragVisualWidget)
            {
                ItemOp->DragVisualWidget->SetActionText(TEXT(""), false);
            }
        }
    }
}

void UPD_ItemSlot::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragLeave(InDragDropEvent, InOperation);

    if (UPD_ItemDragDropOperation* ItemOp = Cast<UPD_ItemDragDropOperation>(InOperation))
    {
        if (ItemOp->DragVisualWidget)
        {
            ItemOp->DragVisualWidget->SetActionText(TEXT(""), false);
        }
    }
}

bool UPD_ItemSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
    if (UPD_ItemDragDropOperation* ItemOp = Cast<UPD_ItemDragDropOperation>(InOperation))
    {
        if (ItemOp->ItemType != this->ItemType)
        {
            UE_LOG(LogTemp, Warning, TEXT("UPD_ItemSlot::NativeOnDrop SwapType"));
            return false;
        }

        if (ItemOp->SourceSlot == this)
        {
            UE_LOG(LogTemp, Warning, TEXT("UPD_ItemSlot::NativeOnDrop SourceSlot"));
            return false;
        }

        APlayerController* PC = GetOwningPlayer();
        if (!PC)
        {
            UE_LOG(LogTemp, Warning, TEXT("UPD_ItemSlot::NativeOnDrop PC"));
            return false;
        }

        APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>();
        if (!PS)
        {
            UE_LOG(LogTemp, Error, TEXT("UPD_ItemSlot::NativeOnDrop PS Null"));
            return false;
        }

        UPDInventoryComponent* Inventory = PS->GetInventoryComponent();
        if (!Inventory)
        {
            UE_LOG(LogTemp, Error, TEXT("UPD_ItemSlot::NativeOnDrop Inventory Null"));
            return false;
        }

        Inventory->SwapItem(
            ItemOp->ItemType, ItemOp->SourceSlot->ItemID, ItemOp->SourceSlot->SlotIndex, ItemOp->SourceSlot->Count,
            this->ItemType, this->ItemID, this->SlotIndex, this->Count
        );
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("UPD_ItemSlot::NativeOnDrop ItemOp"));
    return false;
}
