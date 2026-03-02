#pragma once

#include "CoreMinimal.h"
#include "Object/PDCarriableObjectBase.h"
#include "BallCore.generated.h"

class APawn;
class UStaticMeshComponent;

UCLASS()
class PROJECTD_API ABallCore : public APDCarriableObjectBase
{
	GENERATED_BODY()

public:
	ABallCore();

	virtual void OnInteract_Implementation(AActor* Interactor) override;

	void DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection);

	UStaticMeshComponent* GetStaticMesh() const { return StaticMesh; }

protected:
	virtual void HandleCarrierChanged() override;

};
