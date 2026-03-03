#pragma once

#include "GameplayTagContainer.h"
#include "PDPlayerAbilitySet.generated.h"

class UPDGameplayAbility;

USTRUCT(BlueprintType)
struct FPDPlayerAbilitySet
{
	GENERATED_BODY()

public:
	bool IsValid() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UPDGameplayAbility> AbilityToGrant;
};