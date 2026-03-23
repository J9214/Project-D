// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Skill/GA_PlaceSpawnBase.h"

#include "Components/Combat/SkillManageComponent.h"
#include "Pawn/PDPawnBase.h"
#include "Structs/PDSpawnSkillTableRow.h"

UGA_PlaceSpawnBase::UGA_PlaceSpawnBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_PlaceSpawnBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	if (!IsValid(OwnerPawn))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ActorInfo || !ActorInfo->IsLocallyControlled())
	{
		return;
	}

	USkillManageComponent* SkillManager = OwnerPawn->FindComponentByClass<USkillManageComponent>();
	if (!SkillManager)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FSpawnPlacementData ResolvedPlacementData;
	if (!ResolvePlacementData(ResolvedPlacementData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const bool bStarted = SkillManager->BeginPlacement(this, ResolvedPlacementData);
	if (!bStarted)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_PlaceSpawnBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (!bEndingFromManager)
	{
		if (APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo())
		{
			if (USkillManageComponent* SkillManager = OwnerPawn->FindComponentByClass<USkillManageComponent>())
			{
				SkillManager->CancelPlacementFromAbility(this);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_PlaceSpawnBase::NotifyPlacementFinished(bool bCancelled)
{
	bEndingFromManager = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bCancelled);
	bEndingFromManager = false;
}

bool UGA_PlaceSpawnBase::ResolvePlacementData(FSpawnPlacementData& OutPlacementData) const
{
	if (SpawnDataRowHandle.IsNull())
	{
		return false;
	}

	if (const FPDSpawnSkillTableRow* SpawnRow = SpawnDataRowHandle.GetRow<FPDSpawnSkillTableRow>(
		TEXT("UGA_PlaceSpawnBase::ResolvePlacementData")))
	{
		OutPlacementData = SpawnRow->PlacementData;
		return OutPlacementData.IsValidData();
	}

	return false;
}
