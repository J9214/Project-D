#include "AbilitySystem/Abilities/Player/GA_Dash.h"

#include "AbilitySystemComponent.h"
#include "PDGameplayTags.h"
#include "Components/Input/MovementBridgeComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
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

	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC)
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
	FVector DashDir = OwnerPawn->GetDirectionByMoveInput(OwnerPawn->GetActorForwardVector());
	DashDir.Z = 0.f;
	if (DashDir.IsNearlyZero())
	{
		DashDir = OwnerPawn->GetActorForwardVector();
		DashDir.Z = 0.f;
	}
	DashDir = DashDir.GetSafeNormal();

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
		Target = Hit.Location - (DashDir * CollisionSafetyOffset);
	}
	Target.Z = Start.Z;

	const float ActualDistance = FVector::Dist2D(Start, Target);
	if (bCancelDashIfTooShort && ActualDistance < MinDashTravelDistance)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FVector LaunchDir = (Target - Start);
	LaunchDir.Z = 0.f;
	LaunchDir = LaunchDir.GetSafeNormal();
	if (LaunchDir.IsNearlyZero())
	{
		LaunchDir = DashDir;
	}

	const float Speed = FMath::Max(0.f, DashSpeed);
	const FVector LaunchVelocity = LaunchDir * Speed;

	FMoveRequest Req;
	Req.Type = EMoveRequestType::MoveLaunch;
	Req.Start = Start;
	Req.LaunchVelocity = LaunchVelocity;
	Req.ForceMovementMode = "Falling";
	Req.Priority = Priority;
	Req.bCancelExisting = true;
	Bridge->EnqueueMoveRequest(Req);

	OwnerASC->RemoveLooseGameplayTag(PDGameplayTags::Player_State_DashAvailable);

	UAnimMontage* DashMontage = SelectDashMontage(OwnerPawn, DashDir);
	ExecuteDashCue(DashDir);

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
		else
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		}
}

UAnimMontage* UGA_Dash::SelectDashMontage(const APawn* Pawn, const FVector& MoveDir) const
{
	if (!Pawn)
	{
		return DashMontage_F;
	}

	const FVector Dir = MoveDir.GetSafeNormal();
	const FVector Forward = Pawn->GetActorForwardVector().GetSafeNormal();
	const FVector Right = Pawn->GetActorRightVector().GetSafeNormal();

	const float ForwardDot = FVector::DotProduct(Dir, Forward);
	const float RightDot = FVector::DotProduct(Dir, Right);

	UAnimMontage* SelectedMontage = DashMontage_F;

	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		SelectedMontage = (ForwardDot >= 0.f) ? DashMontage_F : DashMontage_B;
	}
	else
	{
		SelectedMontage = (RightDot >= 0.f) ? DashMontage_R : DashMontage_L;
	}

	return SelectedMontage;
}

void UGA_Dash::ExecuteDashCue(const FVector& DashDir) const
{
	AActor* OwnerActor = GetOwningActorFromActorInfo();
	if (!OwnerActor)
	{
		return;
	}
		
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	FVector Dir = DashDir;
	if (Dir.IsNearlyZero())
	{
		Dir = OwnerActor->GetActorForwardVector();
	}
	Dir = Dir.GetSafeNormal();
	
	static FGameplayTag DashCueTag = PDGameplayTags::GameplayCue_Movement_Dash;

	FGameplayCueParameters Params;
	Params.Normal   = Dir;   

	SourceASC->ExecuteGameplayCue(DashCueTag, Params);
}
