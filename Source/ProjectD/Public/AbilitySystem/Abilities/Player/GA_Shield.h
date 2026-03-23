// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "Skill/PDDamageableSkillActor.h"
#include "GA_Shield.generated.h"

class UMaterialInterface;
class UStaticMesh;

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
	TSubclassOf<APDDamageableSkillActor> ShieldClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Shield")
	TObjectPtr<APDDamageableSkillActor> SpawnedShield = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield")
	bool bAttachShieldToOwner = true;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	FVector ShieldRelativeLocation = FVector(100.f, 0.f, 0.f);
	
	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	FRotator ShieldRelativeRotation = FRotator(0.f, -90.f, 0.f);
	
	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	FVector ShieldScaleVector = FVector(1,1,1);

	
	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	float ShieldMaxHealth = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield")
	EPDShieldType ShieldDamageableType = EPDShieldType::Wall;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	TObjectPtr<UStaticMesh> ShieldStaticMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	TObjectPtr<UMaterialInterface> ShieldBaseMaterial = nullptr;
};
