#include "AbilitySystem/Abilities/Player/GA_Dash.h"
#include "Components/Input/MovementBridgeComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "DefaultMovementSet/LayeredMoves/BasicLayeredMoves.h"
#include "Pawn/PDPawnBase.h"

UGA_Dash::UGA_Dash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Dash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	APDPawnBase* OwnerPawn = GetPlayerPawnFromActorInfo();
	if (!IsValid(OwnerPawn))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UMovementBridgeComponent* Bridge = OwnerPawn->FindComponentByClass<UMovementBridgeComponent>();
	if (!Bridge)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector Start = OwnerPawn->GetActorLocation();
	const FVector DashDir = OwnerPawn->GetDirectionByMoveInput(OwnerPawn->GetActorForwardVector());
	FVector Target = Start + DashDir * DashDistance;
	
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DashSweep), false, OwnerPawn);
	
	const bool bBlocked = OwnerPawn->GetWorld()->SweepSingleByChannel(
		Hit, Start, Target, FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(20.f),
		Params
	);
	
	if (bBlocked)
	{
		Target = Hit.Location;
	}
	
	FMoveRequest Req;
	Req.Type = EMoveRequestType::MoveTo;
	Req.Start = Start;
	Req.Target = Target;
	Req.DurationMs = DashDurationMs; 
	Req.Priority = Priority;
	Bridge->EnqueueMoveRequest(Req);
	
	UAbilityTask_PlayMontageAndWait* PlayTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("DashMontageTask"), DashMontage, 1.0f);
	if (IsValid(PlayTask))
	{
		PlayTask->OnCompleted.AddDynamic(this, &UGA_Dash::OnMontageCompleted);
		PlayTask->OnInterrupted.AddDynamic(this, &UGA_Dash::OnMontageInterrupted);
		PlayTask->OnCancelled.AddDynamic(this, &UGA_Dash::OnMontageCancelled);
		PlayTask->ReadyForActivation();
	}
}

void UGA_Dash::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}



