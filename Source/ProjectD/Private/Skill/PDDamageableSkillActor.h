
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "PDDamageableSkillActor.generated.h"


struct FGameplayEffectSpec;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UAbilitySystemComponent;

UCLASS()
class APDDamageableSkillActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	APDDamageableSkillActor();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category="Shield")
	void InitializeShieldSettings(float InMaxHealth, UStaticMesh* InStaticMesh, UMaterialInterface* InBaseMaterial);


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_ShieldVisuals();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield")
	TObjectPtr<UStaticMeshComponent> ShieldMeshComponent;
	
	UFUNCTION(BlueprintCallable, Category="GAS")
	virtual void OnEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnHitEvent(float DamageAmount,const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="Shield")
	float MaxHealth = 200.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category="Shield")
	float CurrentHealth = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,ReplicatedUsing=OnRep_ShieldVisuals)
	TObjectPtr<UStaticMesh> ShieldStaticMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,ReplicatedUsing=OnRep_ShieldVisuals)
	TObjectPtr<UMaterialInterface> ShieldBaseMaterial = nullptr;

};
