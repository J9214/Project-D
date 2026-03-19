#pragma once

#include "CoreMinimal.h"
#include "Gimmick/PDDirectlyInteractGimmickBase.h"
#include "PDCarriableObjectBase.generated.h"

UCLASS(Abstract)
class PROJECTD_API APDCarriableObjectBase : public APDDirectlyInteractGimmickBase
{
	GENERATED_BODY()
	
public:
	APDCarriableObjectBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetCarrier(APawn* NewCarrier);
	virtual void ClearCarrier();
	virtual void DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection);

	virtual bool IsCanInteract(AActor* Interactor) override;

	void SetPlacedInGoal(bool bInGoal);
protected:
	UFUNCTION()
	void OnRep_CarrierPawn();
	virtual void HandleCarrierChanged() PURE_VIRTUAL(APDCarriableObjectBase::HandleCarrierChanged, );

	UPROPERTY(ReplicatedUsing = OnRep_IsPlacedInGoal)
	bool bIsPlacedInGoal = false;

	UFUNCTION()
	void OnRep_IsPlacedInGoal();
public:
	UPROPERTY(ReplicatedUsing = OnRep_CarrierPawn, BlueprintReadOnly, VisibleInstanceOnly, Category = "Gimmick|Carrier")
	TWeakObjectPtr<APawn> CarrierPawn = nullptr;
};
