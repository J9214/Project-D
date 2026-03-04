#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
#include "GameplayEffect.h"
#include "DroneExplosionFragment.generated.h"

USTRUCT()
struct PROJECTD_API FDroneExplosionFragment : public FMassSharedFragment
{
	GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    float PlayerExplodeRange = 125.0f;

    UPROPERTY(EditAnywhere)
    float ObstacleRange = 25.0f;

    UPROPERTY(EditAnywhere)
    float ExplosionRadius = 300.0f;

    UPROPERTY(EditAnywhere)
    float Damage = 50.0f;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UGameplayEffect> ExplosionDamageGE = nullptr;
};
