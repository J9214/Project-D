#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "AbilitySystem/Abilities/PDGameplayAbility.h"

void UPDAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InInputTag)
{
	if (!InInputTag.IsValid())
	{
		return;
	}
	
	TArray<FGameplayAbilitySpecHandle> HandlesToActivate;
	
	{
		FScopedAbilityListLock AbilityListLock(*this);

		for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
		{
			if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InInputTag))
			{
				continue;
			}

			const UGameplayAbility* AbilityCDO = Spec.Ability;
			const UPDGameplayAbility* PDAbilityCDO = Cast<UPDGameplayAbility>(AbilityCDO);
			if (!AbilityCDO || !PDAbilityCDO)
			{
				continue;
			}
			
			if (PDAbilityCDO->ActivationPolicy == EPDAbilityActivationPolicy::WhileInputActive)
			{
				AbilitySpecInputPressed(Spec);
			}
		
			if (PDAbilityCDO->ActivationPolicy == EPDAbilityActivationPolicy::OnInputToggled)
			{
				if (Spec.IsActive())
				{
					CancelAbilityHandle(Spec.Handle);   
				}
				else
				{
					HandlesToActivate.Add(Spec.Handle); 
				}
			}
			else if (!Spec.IsActive())
			{
				HandlesToActivate.Add(Spec.Handle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& H : HandlesToActivate)
	{
		TryActivateAbility(H);
	}
}

void UPDAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InInputTag)
{
	if (!InInputTag.IsValid())
	{
		return;
	}

	{
		FScopedAbilityListLock AbilityListLock(*this);

		for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
		{
			if (!Spec.GetDynamicSpecSourceTags().HasTagExact(InInputTag))
			{
				continue;
			}

			const UGameplayAbility* AbilityCDO = Spec.Ability;
			const UPDGameplayAbility* PDAbilityCDO = Cast<UPDGameplayAbility>(AbilityCDO);
			if (!AbilityCDO || !PDAbilityCDO)
			{
				continue;
			}
		
			if (PDAbilityCDO->ActivationPolicy == EPDAbilityActivationPolicy::WhileInputActive)
			{
				AbilitySpecInputReleased(Spec);
			}
		}
	}
}