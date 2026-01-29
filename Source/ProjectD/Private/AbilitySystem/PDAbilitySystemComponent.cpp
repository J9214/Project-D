#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/PDGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Pawn/PDPawnBase.h"
#include "Weapon/PDWeaponBase.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "PDGameplayTags.h"

void UPDAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InInputTag)
{
	if (!InInputTag.IsValid())
	{
		return;
	}
	
	TryActivateByInputTag(InInputTag);
	
	if (InInputTag.MatchesTagExact(PDGameplayTags::InputTag_Weapon_Fire))
	{
		if (IsEquippedWeaponFullAuto(this))
		{
			HoldInputTags.Add(InInputTag);
			NextTryTimeByTag.FindOrAdd(InInputTag, 0.f);
		}
	}
}

void UPDAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InInputTag)
{
	if (!InInputTag.IsValid())
	{
		return;
	}
	
	HoldInputTags.Remove(InInputTag);
	NextTryTimeByTag.Remove(InInputTag);
}

void UPDAbilitySystemComponent::TryActivateByInputTag(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}
	
	TArray<FGameplayAbilitySpecHandle> HandlesToActivate;
	{
		FScopedAbilityListLock AbilityListLock(*this);
		for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
		{
			if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
			{
				if (!Spec.IsActive())
				{
					HandlesToActivate.Add(Spec.Handle);
				}
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : HandlesToActivate)
	{
		TryActivateAbility(Handle);
	}
}

void UPDAbilitySystemComponent::ProcessAbilityInput(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (!HoldInputTags.Contains(PDGameplayTags::InputTag_Weapon_Fire))
	{
		return;
	}
	
	if (!IsEquippedWeaponFullAuto(this))
	{
		HoldInputTags.Remove(PDGameplayTags::InputTag_Weapon_Fire);
		NextTryTimeByTag.Remove(PDGameplayTags::InputTag_Weapon_Fire);
		return;
	}

	const float Now = World->GetTimeSeconds();
	float& NextTry = NextTryTimeByTag.FindOrAdd(PDGameplayTags::InputTag_Weapon_Fire, 0.f);

	constexpr float PollInterval = 0.02f; // 50 times per second
	if (Now < NextTry)
	{
		return;
	}
	
	NextTry = Now + PollInterval;

	TryActivateByInputTag(PDGameplayTags::InputTag_Weapon_Fire);
}

APDWeaponBase* UPDAbilitySystemComponent::GetEquippedWeaponFromASC(const UAbilitySystemComponent* ASC) const
{
	if (!ASC)
	{
		return nullptr;
	}
	
	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return nullptr;
	}

	const APDPawnBase* Pawn = Cast<APDPawnBase>(ActorInfo->AvatarActor.Get());
	if (!Pawn)
	{
		return nullptr;
	}
	
	const UWeaponManageComponent* WMC = Pawn->FindComponentByClass<UWeaponManageComponent>();
	
	return WMC ? WMC->GetEquippedWeapon() : nullptr;
}

bool UPDAbilitySystemComponent::IsEquippedWeaponFullAuto(const UAbilitySystemComponent* ASC) const
{
	const APDWeaponBase* Weapon = GetEquippedWeaponFromASC(ASC);
	return (Weapon && Weapon->IsFullAuto());
}