#include "Structs/PDPlayerAbilitySet.h"
#include "AbilitySystem/Abilities/PDGameplayAbility.h"

bool FPDPlayerAbilitySet::IsValid() const
{
	return InputTag.IsValid() && AbilityToGrant;
}
