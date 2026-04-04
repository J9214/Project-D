#include "DataAssets/StartUp/DataAsset_StartUpBase.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/PDGameplayAbility.h"

void UDataAsset_StartUpBase::GiveToAbilitySystemComponent(UPDAbilitySystemComponent* InASC, int32 ApplyLevel)
{
	if (IsValid(InASC))
	{
		GrantAbilities(ActivateOnGivenAbilities, InASC, ApplyLevel);
		GrantAbilities(ReactiveAbilities, InASC, ApplyLevel);
	}
}

void UDataAsset_StartUpBase::GrantAbilities(
	TArray<TSubclassOf<UPDGameplayAbility>>& InAbilitiesToGive,
	UPDAbilitySystemComponent* InASC,
	int32 ApplyLevel)
{
	if (InAbilitiesToGive.IsEmpty())
	{
		return;
	}

	for (const TSubclassOf<UPDGameplayAbility>& Ability : InAbilitiesToGive)
	{
		if (!Ability)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.SourceObject = InASC->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;

		InASC->GiveAbility(AbilitySpec);
	}
}
