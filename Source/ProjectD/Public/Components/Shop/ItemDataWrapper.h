// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FPDItemInfo.h"
#include "ItemDataWrapper.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTD_API UItemDataWrapper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName ItemID;
};
