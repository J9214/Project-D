#include "AbilitySystem/Abilities/Player/GA_SpeedUp.h"
#include "MoverComponent.h"
#include "Structs/FSpeedUpModifier.h"
#include "AbilitySystemComponent.h"
#include "Pawn/PDPawnBase.h"

UGA_SpeedUp::UGA_SpeedUp()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

void UGA_SpeedUp::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	if (!SpeedUpEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SpeedUpEffectClass, GetAbilityLevel(), EffectContext);
	if (!SpecHandle.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UMoverComponent* MoverComp = OwnerPawn->FindComponentByClass<UMoverComponent>();
	if (!MoverComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	TSharedPtr<FSpeedUpModifier> SpeedUpMod = MakeShared<FSpeedUpModifier>();
	SpeedUpMod->SpeedMultiplier = SpeedMultiplier;
	SpeedUpMod->DurationMs = DurationSeconds * 1000.0f;
	
	MoverComp->QueueMovementModifier(StaticCastSharedPtr<FMovementModifierBase>(SpeedUpMod));

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
