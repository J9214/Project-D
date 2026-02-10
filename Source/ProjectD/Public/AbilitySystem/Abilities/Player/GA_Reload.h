#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_Reload.generated.h"

UCLASS()
class PROJECTD_API UGA_Reload : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Reload();

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
