#pragma once

#include "CoreMinimal.h"
#include "Gimmick/PDDirectlyInteractGimmickBase.h"
#include "GoalPost.generated.h"

class ABallCore;
class APawn;
class UStaticMeshComponent;

UCLASS()
class PROJECTD_API AGoalPost : public APDDirectlyInteractGimmickBase
{
	GENERATED_BODY()

public:
	AGoalPost();

	virtual void OnInteract_Implementation(AActor* Interactor) override;

public:
	void ResetGoalPost();

protected:
	bool CanPlaceBall(APawn* Pawn, ABallCore* Ball) const;
	void PlaceBall(APawn* Pawn, ABallCore* Ball);
	void StealBall(APawn* Stealer);
	
	void StartHoldTimer();
	void OnHoldComplete();

	UPROPERTY()
	TObjectPtr<ABallCore> PlacedBall = nullptr;

	FTimerHandle HoldTimer;

};
