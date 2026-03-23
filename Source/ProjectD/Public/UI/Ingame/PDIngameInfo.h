// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PDIngameInfo.generated.h"

class UImage;
class UTextBlock;
class APDWeaponBase;
class APDThrowableItemBase;
class UTexture2D;

/**
 * 
 */
UCLASS()
class PROJECTD_API UPDIngameInfo : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeOnInitialized() override;

	void SetActiveWeapon(const FSlateBrush& InBrush, int32 InCurrentBullet, int32 InMaxBullet, int32 InSlotIndex);
	void SetActiveGrenade(const FSlateBrush& InBrush);
	void SetSkillIcon(int32 SkillIndex, const FSlateBrush& InBrush);

protected:

	UFUNCTION()
	void HandleEquippedWeaponChanged(APDWeaponBase* NewWeapon);

	UFUNCTION()
	void HandleEquippedThrowableChanged(APDThrowableItemBase* NewThrowable);

	UTexture2D* GetItemIconTextureByID(const FName& ItemID) const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ActiveWeapon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ActiveGrenada;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Skill1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Skill2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentBullet;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxBullet;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ActiveSlotIndex;
};
