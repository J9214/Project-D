// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Structs/MassPerceptionType.h"
#include "MassPerceptionSubsystem.generated.h"


class UDataAsset_MassPerceptionSettings;
struct FMassPerceptionTuning;

/**
 *
 */
UCLASS(Config = Game, DefaultConfig)
class PROJECTD_API UMassPerceptionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsServerWorld() const;

	const FMassPerceptionTuning* GetTuningOrNull() const;

	void SubmitAimTubeRequest(const FVector& Origin, const FVector& Dir, float Length);
	void CleanExpiredRequests(float NowSeconds);

public:
	FORCEINLINE const UDataAsset_MassPerceptionSettings* GetSettings() const { return Settings; }
	FORCEINLINE bool HasValidSettings() const { return Settings != nullptr; }
	FORCEINLINE const TArray<FMassAimTubeRequest>& GetActiveAimTubes() const { return ActiveAimTubes; }

private:
	UPROPERTY(Config, EditAnywhere, Category = "Mass|Perception")
	TSoftObjectPtr<UDataAsset_MassPerceptionSettings> SettingsAsset;

	UPROPERTY(Transient)
	TObjectPtr<UDataAsset_MassPerceptionSettings> Settings = nullptr;

	TArray<FMassAimTubeRequest> ActiveAimTubes;

private:
	void LoadSettingsSync();

	int32 GetMaxRequests() const;

	void TrimOneOldestRequest();
};
