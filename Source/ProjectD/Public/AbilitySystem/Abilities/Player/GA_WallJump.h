// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_WallJump.generated.h"

class UAnimMontage;

UCLASS()
class PROJECTD_API UGA_WallJump : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_WallJump();
	
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
	UPROPERTY(EditDefaultsOnly, Category="WallJump")
	float JumpDistance = 50.f;
	
	UPROPERTY(EditDefaultsOnly, Category="WallJump")
	float Radius = 30.f;
	
	UPROPERTY(EditDefaultsOnly, Category="WallJump")
	float HalfHeight = 30.f;
	
	UPROPERTY(EditAnywhere,  Category = "WallJump")
	float WallJumpPushForce = 400.f;
	
	UPROPERTY(EditAnywhere,  Category = "WallJump")
	float WallJumpUpForce = 600.f;
	
	UPROPERTY(EditAnywhere,  Category = "WallJump")
	float WallReflectionIntensity = 1.5f;
	
	UPROPERTY(EditAnywhere,  Category = "WallJump")
	float AirControlInfluence = 0.8f;
	
	UPROPERTY(EditAnywhere, Category="WallJump")
	TObjectPtr<UAnimMontage> JumpMontage;
};
