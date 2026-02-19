#pragma once

#include "CoreMinimal.h"
#include "Object/PDCarriableObjectBase.h"
#include "PDThrowableObject.generated.h"

class UProjectileMovementComponent;

UCLASS()
class PROJECTD_API APDThrowableObject : public APDCarriableObjectBase
{
	GENERATED_BODY()
	
public:
	APDThrowableObject();

public:
	virtual void BeginPlay() override;

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void OnEndInteract_Implementation(AActor* Interactor) override;

	virtual void DropPhysics(const FVector& DropLocation, const FVector& Impulse) override;

	virtual bool IsCanInteract(AActor* Interactor) override;

protected:
	virtual void HandleCarrierChanged() override;

protected:
	void ThrowObject();
	void LoadPDA();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwable|Component")
	TObjectPtr<UProjectileMovementComponent> Projectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Throwable|Data")
	FName PDAType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Throwable|Data")
	FName PDAName;

private:
	static TSet<FName> PDANameSet;
};
