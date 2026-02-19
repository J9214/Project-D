// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassTargetProcessor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTD_API UMassTargetProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMassTargetProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
