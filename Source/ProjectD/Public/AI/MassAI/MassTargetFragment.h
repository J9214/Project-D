// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassTargetFragment.generated.h"

USTRUCT()
struct PROJECTD_API FMassTargetFragment : public FMassFragment
{
    GENERATED_BODY()

public:
    FVector TargetPosition = FVector::ZeroVector;
    bool IsTargetChase = true;
};