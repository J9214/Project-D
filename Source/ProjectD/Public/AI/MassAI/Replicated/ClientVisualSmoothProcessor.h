// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "AI/MassAI/Replicated/ClientVisualFragment.h"
#include "ClientVisualSmoothProcessor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTD_API UClientVisualSmoothProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UClientVisualSmoothProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;

private:
	UPROPERTY(EditAnywhere, Category = "Drone|Visual")
	float LocationInterpSpeed = 18.0f;

	UPROPERTY(EditAnywhere, Category = "Drone|Visual")
	float RotationInterpSpeed = 20.0f;
};
