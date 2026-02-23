// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonTypes.h"
#include "MassEntityTypes.h"
#include "MassBoidsDestructionProcessor.generated.h"

UCLASS()
class PROJECTD_API UMassBoidsDestructionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassBoidsDestructionProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const override { return false; }

private:
	FMassEntityQuery EntityQuery;

private:
	TArray<FMassEntityHandle> PendingDestroyEntities;
};