#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_Dash.generated.h"

class UAnimMontage;

UCLASS()
class PROJECTD_API UGA_Dash : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Dash();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	
public:
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float DashDistance = 600.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float DashDuration = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	int32 Priority = 10;
	
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	TObjectPtr<UAnimMontage> DashMontage;
};
