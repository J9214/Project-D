#pragma once

#include "CoreMinimal.h"
#include "Interface/PDTeamInterface.h"
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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

	UFUNCTION()
	void OnRep_ObjectInfoTeamID();

	void ApplyRoundResetLocal(const FVector& SpawnLocation);
	void UpdateInfoWidgetColor();
	void SetObjectInfoTeamID(ETeamType NewTeamID);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> BallWidget;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UObjectInfo> CachedInfoWidget;

	UPROPERTY(ReplicatedUsing = OnRep_ObjectInfoTeamID, VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	ETeamType ObjectInfoTeamID = ETeamType::None;

private:

	UPROPERTY()
	TObjectPtr<APawn> CachedPlayer;

	ETeamType LastAppliedViewerTeamID = ETeamType::None;

	ETeamType LastAppliedObjectInfoTeamID = ETeamType::None;
};
