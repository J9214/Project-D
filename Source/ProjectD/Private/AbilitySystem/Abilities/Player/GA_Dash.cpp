#include "AbilitySystem/Abilities/Player/GA_Dash.h"

#include "AbilitySystemBlueprintLibrary.h"
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

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
	if (!SourceASC)
	{
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

	DashMontage = SelectDashMontage(OwnerPawn, DashDir);
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
}

UAnimMontage* UGA_Dash::SelectDashMontage(const APawn* Pawn, const FVector& MoveDir) const
{
	if (!Pawn) return DashMontage_F;

	const FVector Dir = MoveDir.GetSafeNormal();
	const FVector Forward = Pawn->GetActorForwardVector().GetSafeNormal();
	const FVector Right = Pawn->GetActorRightVector().GetSafeNormal();

	const float ForwardDot = FVector::DotProduct(Dir, Forward);
	const float RightDot = FVector::DotProduct(Dir, Right);

	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return (ForwardDot >= 0.f) ? DashMontage_F : (DashMontage_B ? DashMontage_B : DashMontage_F);
	}
	else
	{
		return (RightDot >= 0.f)
			       ? (DashMontage_R ? DashMontage_R : DashMontage_F)
			       : (DashMontage_L ? DashMontage_L : DashMontage_F);
	}
}

void UGA_Dash::ExecuteDashCue(const FVector& DashDir) const
{
	AActor* OwnerActor = GetOwningActorFromActorInfo();
	if (!OwnerActor)
	{
		return;
	}
		
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
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
	Params.Location = OwnerActor->GetActorLocation();
	Params.Normal   = Dir;   
	Params.RawMagnitude = DashDistance;          

	SourceASC->ExecuteGameplayCue(DashCueTag, Params);
}
