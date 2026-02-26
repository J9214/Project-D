#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PDThrowableFireArea.generated.h"

class USphereComponent;
class UAbilitySystemComponent;
class UDataAsset_Throwable;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class PROJECTD_API APDThrowableFireArea : public AActor
{
	GENERATED_BODY()

public:
	APDThrowableFireArea();

	void InitFromData(AActor* InOwnerActor, UDataAsset_Throwable* InData);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	UFUNCTION()
	void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	
	void TickDamage();
	void EndArea();
	void CleanupInvalidTargets();
	
	void SpawnFireVFX();
	void CleanupFireVFX();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Collision;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> FireLoopVFX = nullptr;

private:
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TObjectPtr<UDataAsset_Throwable> Data;
	
	UPROPERTY()
	TSet<TWeakObjectPtr<UAbilitySystemComponent>> OverlappingASCs;
	
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> SpawnedFireVFX = nullptr;

	FTimerHandle TickTimerHandle;
	FTimerHandle EndTimerHandle;
};