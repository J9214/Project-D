#pragma once

#include "CoreMinimal.h"
#include "Gimmick/PDDirectlyInteractGimmickBase.h"
#include "GoalPost.generated.h"

class ABallCore;
class APawn;
class UStaticMeshComponent;
class UWidgetComponent;
class UObjectInfo;

UCLASS()
class PROJECTD_API AGoalPost : public APDDirectlyInteractGimmickBase
{
	GENERATED_BODY()

public:
	AGoalPost();

	virtual void BeginPlay() override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	UWidgetComponent* GetGoadWidget() const { return GoalWidget; }
public:
	void ResetGoalPost();

protected:
	bool CanPlaceBall(APawn* Pawn, ABallCore* Ball) const;
	void PlaceBall(APawn* Pawn, ABallCore* Ball);
	void StealBall(APawn* Stealer);
	
	void StartHoldTimer();
	void OnHoldComplete();


protected:
	UPROPERTY()
	TObjectPtr<ABallCore> PlacedBall = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> GoalWidget;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UObjectInfo> CachedInfoWidget;

	FTimerHandle HoldTimer;

	UPROPERTY(BlueprintReadWrite)
	float GoalHoldTime;

private:

	UPROPERTY()
	TObjectPtr<APawn> CachedPlayer;
};
