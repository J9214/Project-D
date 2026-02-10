#include "Components/Combat/WeaponStateComponent.h"
#include "Pawn/PDPawnBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/Player/GA_Fire.h"

UWeaponStateComponent::UWeaponStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SetIsReplicatedByDefault(true);
}

UAbilitySystemComponent* UWeaponStateComponent::GetASC() const
{
	if (const APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner()))
	{
		return OwnerPawn->GetAbilitySystemComponent();
	}

	return nullptr;
}

void UWeaponStateComponent::ForwardTargetDataToFireGA(
	const FGameplayAbilityTargetDataHandle& Data,
	FGameplayTag Tag,
	FGameplayAbilitySpecHandle AbilityHandle,
	FPredictionKey ShotKey
)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	FGameplayAbilitySpec* Spec = FindSpecByHandle(ASC, AbilityHandle);
	if (!Spec)
	{
		return;
	}

	UGameplayAbility* Instance = Spec->GetPrimaryInstance();
	UGA_Fire* FireGA = Cast<UGA_Fire>(Instance);
	if (!FireGA)
	{
		return;
	}

	FireGA->HandleServerReceivedTargetData(Data, Tag, AbilityHandle, ShotKey);
}

FGameplayAbilitySpec* UWeaponStateComponent::FindSpecByHandle(
	UAbilitySystemComponent* ASC,
	FGameplayAbilitySpecHandle Handle
)
{
	if (!ASC)
	{
		return nullptr;
	}
	
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Handle == Handle)
		{
			return &Spec;
		}
	}
	
	return nullptr;
}

void UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation(
	FGameplayAbilitySpecHandle AbilityHandle,
	FPredictionKey ShotKey
)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	if (ASC->AbilityTargetDataSetDelegate(AbilityHandle, ShotKey).IsBound())
	{
		return;
	}

	auto TargetDataReceivedFunc = [this, AbilityHandle, ShotKey](
		const FGameplayAbilityTargetDataHandle& Data,
		FGameplayTag Tag
	)
	{
		UAbilitySystemComponent* LocalASC = GetASC();
		if (!LocalASC)
		{
			return;
		}

		LocalASC->ConsumeClientReplicatedTargetData(AbilityHandle, ShotKey);
		LocalASC->AbilityTargetDataSetDelegate(AbilityHandle, ShotKey).Clear();

		ForwardTargetDataToFireGA(Data, Tag, AbilityHandle, ShotKey);
	};
	ASC->AbilityTargetDataSetDelegate(AbilityHandle, ShotKey).AddLambda(TargetDataReceivedFunc);
	
	ASC->CallReplicatedTargetDataDelegatesIfSet(AbilityHandle, ShotKey);
}
