#pragma once

#include "CoreMinimal.h"
#include "Object/PDCarriableObjectBase.h"
#include "BallCore.generated.h"

class APawn;
class UStaticMeshComponent;
class USphereComponent;
class UPrimitiveComponent;

UCLASS()
class PROJECTD_API ABallCore : public APDCarriableObjectBase
{
	GENERATED_BODY()

public:
	ABallCore();

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection) override;

	void ResetBallForRound(const FVector& SpawnLocation);

	UStaticMeshComponent* GetStaticMesh() const { return StaticMesh; }
	
	void PlaceIntoGoal(USceneComponent* GoalAttachParent);

protected:
	virtual void HandleCarrierChanged() override;
	virtual UPrimitiveComponent* GetPhysicsComponent() const override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyRoundReset(const FVector& SpawnLocation);

	void ApplyRoundResetLocal(const FVector& SpawnLocation);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Sphere;
};
