#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "PDDestructibleObject.generated.h"

class UAbilitySystemComponent;
class AGeometryCollectionActor;
class UPDDestructibleAttributeSet;
class UPDDestructibleObjectAbility;

UCLASS()
class PROJECTD_API APDDestructibleObject : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	APDDestructibleObject();

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPDDestructibleAttributeSet* GetPDDestructibleAttributeSet() const { return AttributeSet; }

public:
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	void InitAbilityActorInfo();
	void InitAttributeSet();
	void BindAttributeSetDelegates();
	void HandleDestroyed();

	UFUNCTION()
	void ChangeToGeometryCollection(AActor* InInstigator, const FVector& HitLocation);

	UFUNCTION()
	void OnRep_Destroyed();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|Component")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|GAS")
	TObjectPtr<UPDDestructibleAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible", ReplicatedUsing = OnRep_Destroyed)
	bool bIsDestroyed;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible")
	TSubclassOf<AGeometryCollectionActor> SpawnGCActor;
};
