// Fill out your copyright notice in the Description page of Project Settings.

#include "Pawn/PDPawnBase.h"
#include "AbilitySystem/Abilities/Player/GA_Shield.h"

#include "AbilitySystemComponent.h"


UGA_Shield::UGA_Shield()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Shield::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData* TriggerEventData)
{
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	if (!IsValid(OwnerPawn))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if(HasAuthority(&ActivationInfo))
	{
		if (SpawnedShield == nullptr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwnerPawn;
			SpawnedShield = GetWorld()->SpawnActor<AActor>(ShieldClass, OwnerPawn->GetActorLocation(),
														   OwnerPawn->GetActorRotation(), SpawnParams);

			if (SpawnedShield)
			{
				FAttachmentTransformRules AttachRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::KeepWorld,
					false
				);

				SpawnedShield->AttachToComponent(OwnerPawn->GetSkeletalMeshComponent(), AttachRules);
				SpawnedShield->SetActorRelativeLocation(ShieldRelativeLocation);
				SpawnedShield->SetActorRelativeRotation(FRotator::ZeroRotator);
			}
		}
	}
}


void UGA_Shield::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (SpawnedShield)
	{
		if (SpawnedShield->HasAuthority())
		{
			SpawnedShield->Destroy();
		}
		SpawnedShield = nullptr;
	}
	Super::OnRemoveAbility(ActorInfo, Spec);
}
