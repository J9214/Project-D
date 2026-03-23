// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/Shop/FPDItemInfo.h"
#include "PD_ItemSlot.generated.h"

class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS()
class PROJECTD_API UPD_ItemSlot : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeOnInitialized() override;

	void UpdateSlot(const FName& NewItemID, int32 NewCount);

	int32 GetSlotIndex() const { return SlotIndex; }
	EItemType GetItemType() const { return ItemType; }
	FName GetItemID() const { return ItemID; }
	int32 GetItemCount() const { return Count; }

protected:

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(EditAnywhere, Category = "ItemSlot|DragVisual")
	TSubclassOf<UUserWidget> DragVisualClass;

	UPROPERTY(EditDefaultsOnly, Category = "ItemSlot|Visual")
	TObjectPtr<UTexture2D> EmptyIconTexture;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDisplayName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;

	UPROPERTY(EditAnywhere, Category = "ItemSlot|ItemType")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, Category = "ItemSlot")
	int32 SlotIndex;

	FName ItemID;

	int32 Count;
};
