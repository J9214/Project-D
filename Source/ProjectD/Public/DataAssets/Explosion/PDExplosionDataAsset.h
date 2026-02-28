#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PDExplosionDataAsset.generated.h"

class UGameplayEffect;
class UFXSystemAsset;

UENUM(BlueprintType)
enum class EExplosionAreaDamageType : uint8
{
	Fire,
	Poison,
	END
};

UCLASS()
class PROJECTD_API UPDExplosionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("Explosion", GetFName());
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	EExplosionAreaDamageType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float Duration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|GAS")
	TSubclassOf<UGameplayEffect> ExplosionDamageGE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|GAS")
	float ExplosionDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|GAS")
	TSubclassOf<UGameplayEffect> ContinuousDamageGE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|GAS")
	float ContinuousDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|GAS")
	TSubclassOf<UGameplayEffect> DestructDamageGE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|GAS")
	float DestructDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Particle")
	UFXSystemAsset* StartEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion|Particle")
	UFXSystemAsset* ContinuousEffectClass;

};
