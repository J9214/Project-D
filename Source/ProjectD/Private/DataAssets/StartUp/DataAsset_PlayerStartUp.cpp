#include "DataAssets/StartUp/DataAsset_PlayerStartUp.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/PDGameplayAbility.h"
#include "GameplayAbilitySpec.h"

void UDataAsset_PlayerStartUp::GiveToAbilitySystemComponent(UPDAbilitySystemComponent* InASC, int32 ApplyLevel)
{
	Super::GiveToAbilitySystemComponent(InASC, ApplyLevel);
	
	for (const FPDPlayerAbilitySet& PlayerAbility : PlayerStartUpAbilitySets)
	{
		if (!PlayerAbility.IsValid())
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(PlayerAbility.AbilityToGrant);
		AbilitySpec.SourceObject = InASC->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(PlayerAbility.InputTag);

		InASC->GiveAbility(AbilitySpec);
	}
}
