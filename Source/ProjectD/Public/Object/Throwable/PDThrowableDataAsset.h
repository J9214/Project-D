#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PDThrowableDataAsset.generated.h"

UCLASS()
class PROJECTD_API UPDThrowableDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("Throw", GetFName());
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float GravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float AdjZVelocity;
};
