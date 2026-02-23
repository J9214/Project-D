// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_Shield.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Shield : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Shield();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	TSubclassOf<AActor> ShieldClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield")
	AActor* SpawnedShield = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	FVector ShieldRelativeLocation = FVector(100.f, 0.f, 0.f);
};

