// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_Inventory.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTD_API UGA_Inventory : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Inventory();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

};
