// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/GA_Shield.h"
#include "Pawn/PDPawnBase.h"
#include "AbilitySystemComponent.h"
#include "Components/SceneComponent.h"
#include "Skill/PDDamageableSkillActor.h"
#include "PlayerState/PDPlayerState.h"


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
			USceneComponent* AttachTarget = OwnerPawn->GetSkeletalMeshComponent();
			if (!AttachTarget)
			{
				AttachTarget = OwnerPawn->GetRootComponent();
			}

			const FTransform AttachTransform = AttachTarget ? AttachTarget->GetComponentTransform() : OwnerPawn->GetActorTransform();
			const FVector ShieldSpawnLocation = AttachTransform.TransformPosition(ShieldRelativeLocation);
			const FRotator ShieldSpawnRotation = OwnerPawn->GetActorRotation() + ShieldRelativeRotation;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwnerPawn;
			SpawnedShield = GetWorld()->SpawnActor<APDDamageableSkillActor>(ShieldClass, ShieldSpawnLocation,
														   ShieldSpawnRotation, SpawnParams);

			if (SpawnedShield)
			{
				ETeamType OwnerTeamID = ETeamType::None;
				if (const APDPlayerState* PS = OwnerPawn->GetPlayerState<APDPlayerState>())
				{
					OwnerTeamID = PS->GetTeamID();
				}

				SpawnedShield->InitializeShieldSettings(
					ShieldMaxHealth,
					ShieldStaticMesh,
					ShieldBaseMaterial,
					OwnerTeamID,
					ShieldDamageableType
				);
				
				if (bAttachShieldToOwner && AttachTarget)
				{
					SpawnedShield->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform);
					SpawnedShield->SetActorRelativeLocation(ShieldRelativeLocation);
					SpawnedShield->SetActorRotation(OwnerPawn->GetActorRotation() + ShieldRelativeRotation);
					SpawnedShield->SetActorRelativeScale3D(ShieldScaleVector);
				}
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
