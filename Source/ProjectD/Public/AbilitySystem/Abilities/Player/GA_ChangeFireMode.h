#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_ChangeFireMode.generated.h"

UCLASS()
class PROJECTD_API UGA_ChangeFireMode : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_ChangeFireMode();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};
