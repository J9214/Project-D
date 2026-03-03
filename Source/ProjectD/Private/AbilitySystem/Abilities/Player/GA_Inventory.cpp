// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_Inventory.h"
#include "Controller/PDPlayerController.h"
#include "UI/HUD/IngameHUD.h"

UGA_Inventory::UGA_Inventory()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UGA_Inventory::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	APDPlayerController* PC = Cast<APDPlayerController>(ActorInfo->PlayerController.Get());
	if (!PC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UIngameHUD* HUD = PC->GetIngameHUDWidget();
	if (HUD)
	{
		UE_LOG(LogTemp, Warning, TEXT("4"));
		HUD->ToggleGameUI();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
