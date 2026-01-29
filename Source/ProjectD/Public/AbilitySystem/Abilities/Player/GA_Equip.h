#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_Equip.generated.h"

class UAnimMontage;

UCLASS()
class PROJECTD_API UGA_Equip : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Equip();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	TObjectPtr<UAnimMontage> EquipMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	FGameplayTag EventTag;
	
private:
	UFUNCTION()
	void OnEventTagReceived(const FGameplayEventData Payload);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Equip", meta = (PrivateAccess = "true"))
	int32 EquipSlotIndex = INDEX_NONE;
};
