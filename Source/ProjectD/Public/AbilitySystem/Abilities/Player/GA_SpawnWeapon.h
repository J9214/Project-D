#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_SpawnWeapon.generated.h"

class APDWeaponBase;

UCLASS()
class PROJECTD_API UGA_SpawnWeapon : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_SpawnWeapon();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<APDWeaponBase> WeaponClass;
};
