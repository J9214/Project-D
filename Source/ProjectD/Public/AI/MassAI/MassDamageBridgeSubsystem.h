#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
#include "MassDamageBridgeSubsystem.generated.h"

class AActor;
struct FHitResult;

USTRUCT()
struct FPendingMassDamage
{
	GENERATED_BODY()

public:
	FMassEntityHandle Entity;
	float Damage = 0.0f;
	TWeakObjectPtr<AActor> Killer = nullptr;
};

UCLASS()
class PROJECTD_API UMassDamageBridgeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool TryApplyDamageFromProxyActor(AActor* HitActor, float Damage, const AActor* DamageCauser = nullptr);
	bool TryApplyDamageFromProxyHit(const FHitResult& Hit, float Damage, const AActor* DamageCauser = nullptr);

	void MovePendingDamages(TArray<FPendingMassDamage>& Out);

private:
	TArray<FPendingMassDamage> PendingDamages;
};
