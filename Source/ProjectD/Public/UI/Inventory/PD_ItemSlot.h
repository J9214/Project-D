// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "ItemSlot|Visual")
	TObjectPtr<UTexture2D> EmptyIconTexture;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDisplayName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Count;

	FName ItemID;
};
