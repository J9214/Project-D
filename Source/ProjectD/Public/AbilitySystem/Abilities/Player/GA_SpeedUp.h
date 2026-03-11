#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_SpeedUp.generated.h"

class UGameplayEffect;

UCLASS()
class PROJECTD_API UGA_SpeedUp : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_SpeedUp();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpeedUp")
	TSubclassOf<UGameplayEffect> SpeedUpEffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SpeedUp")
	float SpeedMultiplier = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SpeedUp")
	float DurationSeconds = 5.0f;
};
