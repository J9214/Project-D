// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PDCharacterCustomInfo.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FPDMutableEnumParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    FString ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    FString SelectedOptionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    int32 RangeIndex = -1;
};

USTRUCT(BlueprintType)
struct FPDMutableFloatParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    FString ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    float Value = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    int32 RangeIndex = -1;
};

USTRUCT(BlueprintType)
struct FPDMutableColorParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    FString ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    FLinearColor Value = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FPDMutableBoolParameter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    FString ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable")
    bool Value = false;
};

USTRUCT(BlueprintType)
struct FPDCharacterCustomInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 CharacterId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable", meta = (TitleProperty = "ParameterName"))
    TArray<FPDMutableEnumParameter> EnumParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable", meta = (TitleProperty = "ParameterName"))
    TArray<FPDMutableFloatParameter> FloatParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable", meta = (TitleProperty = "ParameterName"))
    TArray<FPDMutableColorParameter> ColorParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutable", meta = (TitleProperty = "ParameterName"))
    TArray<FPDMutableBoolParameter> BoolParameters;
};
