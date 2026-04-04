// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PD_DragVisual.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PROJECTD_API UPD_DragVisual : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> DragIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Count;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ActionText;

	void SetActionText(const FString& NewText, bool bVisible);

	UFUNCTION(BlueprintCallable)
	void SetDragVisual(const FSlateBrush& InBrush, int32 InCount);
};
