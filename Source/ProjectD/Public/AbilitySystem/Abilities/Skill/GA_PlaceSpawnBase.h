// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "Structs/SpawnPlacementData.h"
#include "GA_PlaceSpawnBase.generated.h"

class USkillManageComponent;

UCLASS()
class PROJECTD_API UGA_PlaceSpawnBase : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlaceSpawnBase();

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

	UFUNCTION(BlueprintCallable, Category="Placement")
	void NotifyPlacementFinished(bool bCancelled);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement", meta=(RowType="PDSpawnSkillTableRow"))
	FDataTableRowHandle SpawnDataRowHandle;

	bool bEndingFromManager = false;

	bool ResolvePlacementData(FSpawnPlacementData& OutPlacementData) const;
};
