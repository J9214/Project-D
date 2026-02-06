#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmick/PDInteractableObject.h"
#include "BallCore.generated.h"

class APawn;
class UStaticMeshComponent;

UCLASS()
class PROJECTD_API ABallCore : public AActor, public IPDInteractableObject
{
	GENERATED_BODY()

public:
	ABallCore();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;

	void SetCarrier(APawn* NewCarrier);
	void ClearCarrier();
	void DropPhysics(const FVector& DropLocation, const FVector& Impulse);

	UPROPERTY(ReplicatedUsing = OnRep_CarrierPawn, BlueprintReadOnly, VisibleInstanceOnly, Category = "Ball")
	TObjectPtr<APawn> CarrierPawn = nullptr;

protected:
	UFUNCTION()
	void OnRep_CarrierPawn();
	void HandleCarrierChanged();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

};
