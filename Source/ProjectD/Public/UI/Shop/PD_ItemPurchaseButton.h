// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "PD_ItemPurchaseButton.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTD_API UPD_ItemPurchaseButton : public UCommonButtonBase, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual void NativeOnInitialized() override;

	virtual void NativeOnHovered() override;

	virtual void NativeOnUnhovered() override;

	void SetButtonStyle(FLinearColor& Color,bool IsHover = false);

protected:
	UPROPERTY(EditAnywhere, Category = "Appearance|Color")
	FLinearColor NormalColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Appearance|Color")
	FLinearColor HoverColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, Category = "Appearance|Color")
	FLinearColor LowGoldColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, Category = "Appearance|Scale")
	float NomalScale = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Appearance|Scale")
	float HoveredScale = 0.7f;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> OutLine;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDisplayName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Price;

	UFUNCTION()
	void HandleItemPurchase();

	FName ItemID;
};
