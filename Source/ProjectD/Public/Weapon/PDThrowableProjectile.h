#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PDThrowableProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UDataAsset_Throwable;

UCLASS()
class PROJECTD_API APDThrowableProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	APDThrowableProjectile();

	void InitFromData(UDataAsset_Throwable* Data, const FVector& Start, const FVector& Velocity);

protected:
	virtual void BeginPlay() override;
	
private:
	void Explode();
	
	void ApplyGE();
	void SendGameplayCueTag();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
	TObjectPtr<UDataAsset_Throwable> ThrowableData;

	FTimerHandle FuseTimerHandle;

};
