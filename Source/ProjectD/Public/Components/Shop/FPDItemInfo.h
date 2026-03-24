// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"  
#include "FPDItemInfo.generated.h"

class UGameplayAbility;
class AActor;
class UTexture2D;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None = 0,
	Weapon = 1,
	Skill = 2,
	Grenade = 3,
	Etc = 4,
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTD_API FPDItemInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType ItemType = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> ItemAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> IconImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName DisplayName;
};
