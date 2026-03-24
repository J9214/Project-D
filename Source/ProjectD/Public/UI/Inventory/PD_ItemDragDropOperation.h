// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/Shop/FPDItemInfo.h"
#include "PD_ItemDragDropOperation.generated.h"

class UPD_ItemSlot;
class UPD_DragVisual;

/**
 * 
 */

UCLASS()
class PROJECTD_API UPD_ItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	FName ItemID;
	int32 ItemCount;
	EItemType ItemType;

	UPROPERTY()
	TObjectPtr<UPD_ItemSlot> SourceSlot;

	UPROPERTY()
	TObjectPtr<UPD_DragVisual> DragVisualWidget;
};
