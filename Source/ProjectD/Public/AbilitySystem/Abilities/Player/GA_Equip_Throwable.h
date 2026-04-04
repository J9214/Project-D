#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_Equip_Throwable.generated.h"

UCLASS()
class PROJECTD_API UGA_Equip_Throwable : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Equip_Throwable();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

private:
	UFUNCTION()
	void OnEventTagReceived(const FGameplayEventData Payload);
	
private:
	int32 EquipSlotIndex = INDEX_NONE;
};
