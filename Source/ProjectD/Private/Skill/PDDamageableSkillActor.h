
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "PDDamageableSkillActor.generated.h"


struct FGameplayEffectSpec;

UCLASS()
class APDDamageableSkillActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	APDDamageableSkillActor();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UFUNCTION(BlueprintCallable, Category="GAS")
	virtual void OnEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnHitEvent(float DamageAmount,const FHitResult& Hit);

};
