#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DroneExplosionSubsystem.generated.h"

class UGameplayEffect;

UCLASS()
class PROJECTD_API UDroneExplosionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	void ApplyExplosionDamage(const FVector& Location, float Radius, float Damage, TSubclassOf<UGameplayEffect> DamageGE);;

private:
	bool HasLineOfSightToASC(class UAbilitySystemComponent* ASC, const FVector& Origin) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	bool bCheckLineOfSight = false;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	TEnumAsByte<ECollisionChannel> LineOfSightTraceChannel = ECC_GameTraceChannel1;
};
