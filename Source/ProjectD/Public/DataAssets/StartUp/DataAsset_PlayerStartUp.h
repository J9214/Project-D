#pragma once

#include "CoreMinimal.h"
#include "DataAssets/StartUp/DataAsset_StartUpBase.h"
#include "Structs/PDPlayerAbilitySet.h"
#include "DataAsset_PlayerStartUp.generated.h"

class UPDAbilitySystemComponent;

UCLASS()
class PROJECTD_API UDataAsset_PlayerStartUp : public UDataAsset_StartUpBase
{
	GENERATED_BODY()
public:
	virtual void GiveToAbilitySystemComponent(UPDAbilitySystemComponent* InASC, int32 ApplyLevel = 1) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "StartUpData", meta = (TitleProperty = "InputTag"))
	TArray<FPDPlayerAbilitySet> PlayerStartUpAbilitySets;
};
