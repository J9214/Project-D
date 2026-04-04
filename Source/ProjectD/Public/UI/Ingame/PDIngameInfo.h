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

	void SetActiveWeapon(const FSlateBrush& InBrush, int32 InCurrentAmmo, int32 InMaxAmmo, int32 InSlotIndex);
	void SetActiveGrenade(const FSlateBrush& InBrush);
	void SetSkillIcon(int32 SkillIndex, const FSlateBrush& InBrush);

	void UpdateCurrentAmmo(int32 AmmoCount);

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
	TObjectPtr<UTextBlock> CurrentAmmo;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxAmmo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ActiveSlotIndex;

	UPROPERTY(EditDefaultsOnly, Category = "UPDIngameInfo|Visual")
	TObjectPtr<UTexture2D> EmptyWeaponIconTexture;
	UPROPERTY(EditDefaultsOnly, Category = "UPDIngameInfo|Visual")
	TObjectPtr<UTexture2D> EmptyThrowIconTexture;
	UPROPERTY(EditDefaultsOnly, Category = "UPDIngameInfo|Visual")
	TObjectPtr<UTexture2D> EmptySkillIconTexture;

};
