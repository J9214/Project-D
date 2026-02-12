#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PDDestructibleAttributeSet.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDurabilityChanged, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOutOfDurabilityChanged, AActor*, Instigator, const FVector&, HitLocation);

UCLASS()
class PROJECTD_API UPDDestructibleAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPDDestructibleAttributeSet();

	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, MaxDurability);
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Durability);
	ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Damaged);

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category = "Destructible|Attribute")
	mutable FDurabilityChanged OnMaxDurabilityChanged;

	/*UPROPERTY(BlueprintAssignable, Category = "Destructible|Attribute")
	mutable FDurabilityChanged OnDurabilityChanged;*/

	UPROPERTY(BlueprintAssignable, Category = "Destructible|Attribute")
	mutable FOutOfDurabilityChanged OutOfDurabilityChanged;

private:
	UFUNCTION()
	void OnRep_MaxDurability(const FGameplayAttributeData& OldMaxDurability);

	UFUNCTION()
	void OnRep_Durability(const FGameplayAttributeData& OldDurability);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Destructible|Attribute", ReplicatedUsing = OnRep_MaxDurability)
	FGameplayAttributeData MaxDurability;

	UPROPERTY(BlueprintReadOnly, Category = "Destructible|Attribute", ReplicatedUsing = OnRep_Durability)
	FGameplayAttributeData Durability;

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData Damaged;

	const static float MIN_DURABILITY;
};
