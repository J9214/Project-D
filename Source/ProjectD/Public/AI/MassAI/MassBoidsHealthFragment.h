// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassBoidsHealthFragment.generated.h"

/**
 * PROJECTD_API
 */
USTRUCT()
struct PROJECTD_API FMassBoidsHealthFragment : public FMassFragment
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float Health = 100.0f;

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.0f;
};