#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
#include "MassDamageBridgeSubsystem.generated.h"

struct FHitResult;

USTRUCT()
struct FPendingMassDamage
{
	GENERATED_BODY()

public:
	FMassEntityHandle Entity;
	float Damage = 0.0f;
};

UCLASS()
class PROJECTD_API UMassDamageBridgeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool TryApplyDamageFromProxyActor(AActor* HitActor, float Damage);
	bool TryApplyDamageFromProxyHit(const FHitResult& Hit, float Damage);

	void MovePendingDamages(TArray<FPendingMassDamage>& Out);

private:
	TArray<FPendingMassDamage> PendingDamages;
};
