// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PDAttributeSetBindProxy.generated.h"

class UIngameHUD;
class UPDAttributeSetBase;

UENUM(BlueprintType)
enum class EHPBarSlot : uint8
{
	Player,
	Team1,
};
/**
 * 
 */
UCLASS()
class PROJECTD_API UPDAttributeSetBindProxy : public UObject
{
	GENERATED_BODY()

public:

    void Init(class UIngameHUD* InHUD, EHPBarSlot InSlot, UPDAttributeSetBase* InSet);

    void Unbind();

    UFUNCTION()
    void OnHealth(float OldValue, float NewValue);

    UFUNCTION()
    void OnMaxHealth(float OldValue, float NewValue);

private:

    UPROPERTY()
    TObjectPtr<UIngameHUD> HUD;

    UPROPERTY()
    TObjectPtr<UPDAttributeSetBase> BindSet;

    UPROPERTY()
    EHPBarSlot Slot;
};
