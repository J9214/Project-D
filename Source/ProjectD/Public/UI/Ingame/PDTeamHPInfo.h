// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Interface/PDTeamInterface.h"
#include "PDTeamHPInfo.generated.h"

class UImage;
class UTextBlock;
class UWidgetAnimation;
/**
 * 
 */
UCLASS()
class PROJECTD_API UPDTeamHPInfo : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool CheckInit() { return IsInit; }

	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void Init(const FString& DisplayName);

	UFUNCTION()
	float HandleHealthChanged(float OldValue, float NewValue);

	UFUNCTION(BlueprintCallable)
	void SetPlayerColor();

	UFUNCTION(BlueprintCallable)
	void SetTeamColor(bool TeamType);

	UFUNCTION(BlueprintCallable)
	void SetTeamTextColor(ETeamType LocalTeamID, ETeamType TargetTeamID);

protected:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedBarFillMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedBarGlowMID;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BarFill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BarGlow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NickName;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Damaged;

	UPROPERTY()
	float NowHPValue;
	UPROPERTY()
	float MaxHPValue;
	
	bool IsInit = false;
};
