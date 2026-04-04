#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_Flash.generated.h"

class UDataAsset_Throwable;

UCLASS()
class PROJECTD_API UGA_Flash : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Flash();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

private:
	UFUNCTION()
	void OnFlashTargetDataReady(
		const FGameplayAbilityTargetDataHandle& DataHandle,
		FGameplayTag ActivationTag
	);
	
	void SendFlashTargetData(const FHitResult& HitResult);

	bool SpawnFlashProjectileFromData(
		APDPawnBase* OwnerPawn,
		const FGameplayAbilityTargetDataHandle& DataHandle
	);
	
	bool SpawnFlashProjectile(APDPawnBase* OwnerPawn);

private:
	UPROPERTY(EditDefaultsOnly, Category="Flash")
	TObjectPtr<UDataAsset_Throwable> FlashData = nullptr;
};
