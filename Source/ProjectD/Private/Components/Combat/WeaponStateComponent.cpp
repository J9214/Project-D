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

	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(AbilityHandle);
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

FGameplayAbilitySpec* UWeaponStateComponent::FindSpecByTag(
	UAbilitySystemComponent* ASC,
	const FGameplayTag& Tag
)
{
	if (!ASC || !Tag.IsValid())
	{
		return nullptr;
	}
	
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(Tag))
		{
			return &Spec;
		}
	}
	
	return nullptr;
}

void UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation(
	FGameplayTag FireTag,
	FPredictionKey ShotKey
)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}
	
	if (!FireTag.IsValid() || !ShotKey.IsValidKey())
	{
		return;
	}
	
	FGameplayAbilitySpec* Spec = FindSpecByTag(ASC, FireTag);
	if (!Spec)
	{
		return;
	}
	
	const FGameplayAbilitySpecHandle ServerHandle = Spec->Handle;

	auto& Delegate = ASC->AbilityTargetDataSetDelegate(ServerHandle, ShotKey);
	if (Delegate.IsBound())
	{
		return;
	}

	TWeakObjectPtr<UWeaponStateComponent> WeakThis(this);
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);

	Delegate.AddLambda([WeakThis, WeakASC, ServerHandle, ShotKey](const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Tag)
	{
		if (!WeakThis.IsValid() || !WeakASC.IsValid())
		{
			return;
		}

		UWeaponStateComponent* This = WeakThis.Get();
		UAbilitySystemComponent* LocalASC = WeakASC.Get();

		if (!LocalASC->FindAbilitySpecFromHandle(ServerHandle))
		{
			return;
		}

		LocalASC->ConsumeClientReplicatedTargetData(ServerHandle, ShotKey);
		LocalASC->AbilityTargetDataSetDelegate(ServerHandle, ShotKey).Clear();

		This->ForwardTargetDataToFireGA(Data, Tag, ServerHandle, ShotKey);
	});

	ASC->CallReplicatedTargetDataDelegatesIfSet(ServerHandle, ShotKey);
}
