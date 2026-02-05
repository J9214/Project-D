// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassBoidsDestructionProcessor.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS()
class PROJECTD_API UMassBoidsDestructionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassBoidsDestructionProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;

public:
	UPROPERTY(EditAnywhere, Category = "FX")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditAnywhere, Category = "FX")
	TObjectPtr<USoundBase> ExplosionSound;

	UPROPERTY(EditAnywhere, Category = "FX")
	FVector EffectScale = FVector(1.0f);
};