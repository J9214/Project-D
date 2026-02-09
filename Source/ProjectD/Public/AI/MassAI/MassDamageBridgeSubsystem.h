#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassDamageBridgeSubsystem.generated.h"

struct FHitResult;

UCLASS()
class PROJECTD_API UMassDamageBridgeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool TryApplyDamageFromProxyActor(AActor* HitActor, float Damage);
	bool TryApplyDamageFromProxyHit(const FHitResult& Hit, float Damage);
};
