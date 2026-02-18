// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonTypes.h"
#include "MassBoidsDestructionProcessor.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS(Config = Game, DefaultConfig)
class PROJECTD_API UMassBoidsDestructionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassBoidsDestructionProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const override { return false; }

private:
	void SpawnDeathFX(const FVector& DeathLocation) const;

private:
	FMassEntityQuery EntityQuery;

private:
	UPROPERTY(Config, EditAnywhere, Category = "FX")
	TSoftObjectPtr<UNiagaraSystem> ExplosionEffectAsset;

	UPROPERTY(Config, EditAnywhere, Category = "FX")
	TSoftObjectPtr<USoundBase> ExplosionSoundAsset;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> ExplosionEffect = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> ExplosionSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "FX")
	FVector EffectScale = FVector(1.0f);

	TArray<FMassNetworkID> PendingDeathNetIDs;
	TArray<FMassEntityHandle> PendingDestroyEntities;
};