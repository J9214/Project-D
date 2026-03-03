#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayPrediction.h"
#include "GameplayTagContainer.h"
#include "WeaponStateComponent.generated.h"

class UAbilitySystemComponent;
struct FGameplayAbilitySpec;

UCLASS()
class PROJECTD_API UWeaponStateComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:
	UWeaponStateComponent();

	UFUNCTION(Server, Reliable)
	void ServerRPC_RegisterFireShotKey(FGameplayTag FireTag, FPredictionKey ShotKey);
	
protected:
	UAbilitySystemComponent* GetASC() const;

	void ForwardTargetDataToFireGA(
		const FGameplayAbilityTargetDataHandle& Data,
		FGameplayTag Tag,
		FGameplayAbilitySpecHandle AbilityHandle,
		FPredictionKey ShotKey
	);

	FGameplayAbilitySpec* FindSpecByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag);
};
