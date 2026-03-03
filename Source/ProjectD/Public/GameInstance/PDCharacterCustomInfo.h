// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PDCharacterCustomInfo.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FPDCharacterCustomInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 CharacterId = 0;
};
