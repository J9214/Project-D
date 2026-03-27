#pragma once

#include "CoreMinimal.h"
#include "Object/PDCarriableObjectBase.h"
#include "BallCore.generated.h"

class APawn;
class UStaticMeshComponent;
class USphereComponent;
class UPrimitiveComponent; 
class UWidgetComponent;
class UObjectInfo;

UCLASS()
class PROJECTD_API ABallCore : public APDCarriableObjectBase
{
	GENERATED_BODY()

public:
	ABallCore();

	virtual void BeginPlay() override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection) override;
	virtual void Tick(float DeltaTime) override;

	void ResetBallForRound(const FVector& SpawnLocation);

	UStaticMeshComponent* GetStaticMesh() const { return StaticMesh; }
	
	void PlaceIntoGoal(USceneComponent* GoalAttachParent);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UWidgetComponent* GetBallWidget() const { return BallWidget; }
protected:
	virtual void HandleCarrierChanged() override;
	virtual UPrimitiveComponent* GetPhysicsComponent() const override;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyRoundReset(const FVector& SpawnLocation);

	void ApplyRoundResetLocal(const FVector& SpawnLocation);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> BallWidget;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UObjectInfo> CachedInfoWidget;

private:

	UPROPERTY()
	TObjectPtr<APawn> CachedPlayer;
};
