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
	UE_LOG(LogTemp, Warning, TEXT("UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation - FireTag: %s, ShotKey: %s"), *FireTag.ToString(), *ShotKey.ToString());
	
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation - ASC is not valid"));
		return;
	}
	
	if (!FireTag.IsValid() || !ShotKey.IsValidKey())
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation - Invalid FireTag or ShotKey"));
		return;
	}
	
	FGameplayAbilitySpec* Spec = FindSpecByTag(ASC, FireTag);
	if (!Spec)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation - No Ability Spec found with Tag: %s"), *FireTag.ToString());
		return;
	}
	
	const FGameplayAbilitySpecHandle ServerHandle = Spec->Handle;

	auto& Delegate = ASC->AbilityTargetDataSetDelegate(ServerHandle, ShotKey);
	if (Delegate.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation - Delegate already bound for Handle: %s, ShotKey: %s"), *ServerHandle.ToString(), *ShotKey.ToString());
		return;
	}

	TWeakObjectPtr<UWeaponStateComponent> WeakThis(this);
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);

	Delegate.AddLambda([WeakThis, WeakASC, ServerHandle, ShotKey](const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Tag)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation - Delegate Lambda Invoked for Handle: %s, ShotKey: %s"), *ServerHandle.ToString(), *ShotKey.ToString());
		
		if (!WeakThis.IsValid() || !WeakASC.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation - Weak Pointers are not valid"));
			return;
		}

		UWeaponStateComponent* This = WeakThis.Get();
		UAbilitySystemComponent* LocalASC = WeakASC.Get();

		if (!LocalASC->FindAbilitySpecFromHandle(ServerHandle))
		{
			UE_LOG(LogTemp, Warning, TEXT("UWeaponStateComponent::ServerRPC_RegisterFireShotKey_Implementation - Ability Spec not found for handle: %s"), *ServerHandle.ToString());
			return;
		}

		LocalASC->ConsumeClientReplicatedTargetData(ServerHandle, ShotKey);
		LocalASC->AbilityTargetDataSetDelegate(ServerHandle, ShotKey).Clear();

		This->ForwardTargetDataToFireGA(Data, Tag, ServerHandle, ShotKey);
	});

	ASC->CallReplicatedTargetDataDelegatesIfSet(ServerHandle, ShotKey);
}
