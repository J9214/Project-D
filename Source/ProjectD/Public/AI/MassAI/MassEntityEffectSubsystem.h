#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AI/MassAI/MassEntityCueId.h"
#include "MassEntityEffectSubsystem.generated.h"

class UNiagaraSystem;
class USoundBase;

USTRUCT()
struct PROJECTD_API FMassEntityEffectCue
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Effect")
	EMassEntityCueId CueId = EMassEntityCueId::None;

	UPROPERTY(EditAnywhere, Config, Category = "Effect")
	TSoftObjectPtr<UNiagaraSystem> NiagaraEffectAsset;

	UPROPERTY(EditAnywhere, Config, Category = "Effect")
	TSoftObjectPtr<USoundBase> SoundAsset;

	UPROPERTY(EditAnywhere, Config, Category = "Effect")
	FVector EffectScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, Config, Category = "Effect")
	bool bPlaySound = true;

	UPROPERTY(EditAnywhere, Config, Category = "Effect")
	bool bPlayNiagara = true;
};

UCLASS(Config = Game, DefaultConfig)
class PROJECTD_API UMassEntityEffectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	void PlayCueAtLocation(const EMassEntityCueId CueId, const FVector& Location);

private:
	const FMassEntityEffectCue* FindCueDef(const EMassEntityCueId CueId) const;

private:
	struct FCachedCue
	{
		TObjectPtr<UNiagaraSystem> Niagara = nullptr;
		TObjectPtr<USoundBase> Sound = nullptr;
		FVector Scale = FVector(1.0f);
		bool bPlaySound = true;
		bool bPlayNiagara = true;
	};

private:
	UPROPERTY(EditAnywhere, Config, Category = "Effect")
	TArray<FMassEntityEffectCue> Cues;

private:
	TMap<EMassEntityCueId, FCachedCue> CachedCues;
};
