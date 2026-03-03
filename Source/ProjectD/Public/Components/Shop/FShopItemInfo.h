// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"  
#include "FShopItemInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTD_API FShopItemInfo : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ItemType = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price = 0;


};
