#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PDThrowableSmokeArea.generated.h"

class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UDataAsset_Throwable;
class UAbilitySystemComponent;

UCLASS()
class PROJECTD_API APDThrowableSmokeArea : public AActor
{
	GENERATED_BODY()

public:
	APDThrowableSmokeArea();
	
	void InitFromData(AActor* InOwnerActor, UDataAsset_Throwable* InData);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void EndArea();

	void SpawnSmokeVFX();
	void CleanupSmokeVFX();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> SmokeLoopVFX = nullptr;
	
private:
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TObjectPtr<UDataAsset_Throwable> Data;

	FTimerHandle EndTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> SpawnedSmokeVFX = nullptr;

	UPROPERTY()
	TSet<TWeakObjectPtr<UAbilitySystemComponent>> OverlappingASCs;
};