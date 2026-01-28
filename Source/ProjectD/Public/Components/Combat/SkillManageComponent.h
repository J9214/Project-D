#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "SkillManageComponent.generated.h"

USTRUCT(BlueprintType)
struct FSkillSlot
{
	GENERATED_BODY()

public:
	bool IsEmpty() const;
	
public:
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotTag;
};

UCLASS()
class PROJECTD_API USkillManageComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:
	USkillManageComponent();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Skill")
	void ServerGrantSkillToSlot(TSubclassOf<UGameplayAbility> AbilityClass, FGameplayTag SlotTag);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Skill")
	void ServerRemoveSkillFromSlot(FGameplayTag SlotTag);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_Slots();

	UAbilitySystemComponent* GetASC() const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Slots)
	FSkillSlot Slots[2];

	FGameplayAbilitySpecHandle SlotHandles[2];
};
