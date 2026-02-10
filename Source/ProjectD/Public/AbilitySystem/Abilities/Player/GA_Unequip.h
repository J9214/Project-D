#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_Unequip.generated.h"

UCLASS()
class PROJECTD_API UGA_Unequip : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Unequip();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
private:
	UFUNCTION()
	void OnEventTagReceived(const FGameplayEventData Payload);
};
