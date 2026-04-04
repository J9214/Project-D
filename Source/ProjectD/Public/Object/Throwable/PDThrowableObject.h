#pragma once

#include "CoreMinimal.h"
#include "Object/PDCarriableObjectBase.h"
#include "PDThrowableObject.generated.h"

class UProjectileMovementComponent;
class UCapsuleComponent;
class APDExplosionActor;

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

	virtual void DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection) override;

	virtual bool IsCanInteract(AActor* Interactor) override;

protected:
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleCarrierChanged() override;

	UFUNCTION()
	void HandleHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCollisionEnabled(ECollisionEnabled::Type InType);

protected:
	void ThrowObject();
	void Explode(const FVector& HitLocation);
	void LoadPDA();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwable|Component")
	TObjectPtr<UProjectileMovementComponent> Projectile;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwable|Component")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable|Explosion")
	TSubclassOf<APDExplosionActor> ExplosionClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Throwable|Data")
	FName PDAType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Throwable|Data")
	FName PDAName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Throwable|Data")
	FName PDAExplosionType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Throwable|Data")
	FName PDAExplosionName;

	FVector CamDirection;

private:
	static TSet<FName> PDANameSet;
	static TSet<FName> PDAExplosionNameSet;
};
