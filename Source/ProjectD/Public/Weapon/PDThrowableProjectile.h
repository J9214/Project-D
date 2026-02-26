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
	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
	
	void Explode();
	
	void ApplyExplosionGE();
	
	void SpawnFireArea();
	void SpawnSmokeArea();
	
	void SendExplosionCueTag();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMove;
	
private:
	UPROPERTY()
	TObjectPtr<UDataAsset_Throwable> ThrowableData;
	
	UPROPERTY()
	bool bExploded = false;
	
	UPROPERTY()
	FVector CachedExplosionLocation = FVector::ZeroVector;
	
	bool bExplosionOnImpact = false;
	
	FTimerHandle FuseTimerHandle;
};
