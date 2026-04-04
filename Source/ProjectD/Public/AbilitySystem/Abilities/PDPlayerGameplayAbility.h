#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDGameplayAbility.h"
#include "PDPlayerGameplayAbility.generated.h"

class APDPawnBase;
class APDPlayerController;

UCLASS()
class PROJECTD_API UPDPlayerGameplayAbility : public UPDGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "MA|Ability")
	APDPawnBase* GetPlayerPawnFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "MA|Ability")
	APDPlayerController* GetPlayerControllerFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "MA|Ability")
	UWeaponManageComponent* GetWeaponManageComponentFromActorInfo();

private:
	TWeakObjectPtr<APDPawnBase> CachedPlayerPawn;
	TWeakObjectPtr<APDPlayerController> CachedPlayerController;
};
