#include "AbilitySystem/Abilities/Player/GA_WallJump.h"
#include "Components/Input/MovementBridgeComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Pawn/PDPawnBase.h"
#include "PDGameplayTags.h"

UGA_WallJump::UGA_WallJump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_WallJump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	if (!IsValid(OwnerPawn))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UMovementBridgeComponent* Bridge = OwnerPawn->FindComponentByClass<UMovementBridgeComponent>();
	if (!Bridge)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	const FVector Start = OwnerPawn->GetActorLocation();
	const FVector DashDir = OwnerPawn->GetDirectionByMoveInput(OwnerPawn->GetActorForwardVector());
	FVector End = Start + (DashDir * JumpDistance);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerPawn);

	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DashSweep), false, OwnerPawn);

	const bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(
		GetWorld(),
		Start,
		End,
		Radius,
		HalfHeight,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
	);

	if (bHit)
	{
		FVector ImpactNormal = HitResult.ImpactNormal;
		FVector MoveInput = OwnerPawn->GetDirectionByMoveInput(OwnerPawn->GetActorForwardVector());
		FVector ReflectionSide = ImpactNormal * WallReflectionIntensity;
		FVector InputSide = MoveInput * AirControlInfluence;
		FVector LaunchDirection = (ReflectionSide + InputSide).GetSafeNormal();

		FMoveRequest Req;
		Req.Type = EMoveRequestType::MoveLaunch;
		Req.Start = Start;
		Req.LaunchVelocity.X = LaunchDirection.X * WallJumpPushForce;
		Req.LaunchVelocity.Y = LaunchDirection.Y * WallJumpPushForce;
		Req.LaunchVelocity.Z = WallJumpUpForce;
		Req.ForceMovementMode = "Falling";

		Bridge->EnqueueMoveRequest(Req);

		if (UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo())
		{
			if (!OwnerASC->HasMatchingGameplayTag(PDGameplayTags::Player_State_DashAvailable))
			{
				OwnerASC->AddLooseGameplayTag(PDGameplayTags::Player_State_DashAvailable);
			}
		}

		UAbilityTask_PlayMontageAndWait* PlayTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, TEXT("WallJumpMontageTask"), JumpMontage, 1.0f);
		if (IsValid(PlayTask))
		{
			PlayTask->OnCompleted.AddDynamic(this, &UGA_WallJump::OnMontageCompleted);
			PlayTask->OnInterrupted.AddDynamic(this, &UGA_WallJump::OnMontageInterrupted);
			PlayTask->OnCancelled.AddDynamic(this, &UGA_WallJump::OnMontageCancelled);
			PlayTask->ReadyForActivation();
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
