#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class APDDirectlyInteractGimmickBase;
class UCameraComponent;
class UCapsuleComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTD_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool CanInteract() const { return bCanInteract; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	APDDirectlyInteractGimmickBase* GetCurrentInteractTarget() const { return CurrentInteractTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void EndInteract();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void RefreshInteractState();
	void BuildInteractionCapsule();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionLength = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionRadius = 30.f;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bCanInteract = false;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TWeakObjectPtr<APDDirectlyInteractGimmickBase> CurrentInteractTarget;

	UPROPERTY(Transient)
	TObjectPtr<UCapsuleComponent> InteractionCapsule;

	TSet<TWeakObjectPtr<APDDirectlyInteractGimmickBase>> OverlappedTargets;
};
