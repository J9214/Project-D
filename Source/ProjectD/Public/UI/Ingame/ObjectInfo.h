// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ObjectInfo.generated.h"

class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS()
class PROJECTD_API UObjectInfo : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetInterfaceColor(FLinearColor NewColor);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetFillAmount(float Percent);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateDistanceUI(int32 InDistance);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<UImage> InfoImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TObjectPtr<UImage> FillImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DistanceText;

private:
	bool bCurrentVisible = true;
	int32 Distance = 0;
};
