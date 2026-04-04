#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset_StartUpBase.generated.h"

class UPDAbilitySystemComponent;
class UPDGameplayAbility;

UCLASS()
class PROJECTD_API UDataAsset_StartUpBase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual void GiveToAbilitySystemComponent(UPDAbilitySystemComponent* InASC, int32 ApplyLevel = 1);
	
protected:
	void GrantAbilities(TArray<TSubclassOf<UPDGameplayAbility>>& InAbilitiesToGive, UPDAbilitySystemComponent* InASC, int32 ApplyLevel = 1);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "StartUpData")
	TArray<TSubclassOf<UPDGameplayAbility>> ActivateOnGivenAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "StartUpData")
	TArray<TSubclassOf<UPDGameplayAbility>> ReactiveAbilities;
};
