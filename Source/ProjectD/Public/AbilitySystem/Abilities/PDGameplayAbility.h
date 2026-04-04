#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PDGameplayAbility.generated.h"

class UPDAbilitySystemComponent;

UENUM(BlueprintType)
enum class EPDAbilityActivationPolicy : uint8
{
	OnInputTriggered,
	WhileInputActive,
	OnInputToggled,
	OnGiven
};

UCLASS()
class PROJECTD_API UPDGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void OnGiveAbility(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilitySpec& Spec
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION(BlueprintPure, Category = "Ability")
	UPDAbilitySystemComponent* GetPDAbilitySystemComponentFromActorInfo() const;
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	EPDAbilityActivationPolicy ActivationPolicy = EPDAbilityActivationPolicy::OnInputTriggered;
};
