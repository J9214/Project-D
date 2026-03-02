#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataAssets/Explosion/PDExplosionDataAsset.h"
#include "AbilitySystemComponent.h"
#include "PDExplosionActor.generated.h"

class USphereComponent;
class UFXSystemAsset;
class UFXSystemComponent;
class UGameplayEffect;

UCLASS()
class PROJECTD_API APDExplosionActor : public AActor
{
	GENERATED_BODY()
	
public:
	APDExplosionActor();

	void InitExplosion(FName InName, FName InType = TEXT("Explosion"));
	void OnPDAReady();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	void LoadPDA(FName InType, FName InName);

	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowExplodeEffect(UFXSystemAsset* InStartParticle, UFXSystemAsset* InContinuousParticle, const float& InRange, const float& InDuration);
	void CreateParticle(UFXSystemAsset* InParticle, bool bIsContinuous = false);

	void BindOverlapHandles();

	bool IsCanTakeDamage(UAbilitySystemComponent* ASC);
	void Explode();
	void ApplyEffect(UAbilitySystemComponent* ASC, bool bIsExplode = false);
	void TimerOn(UAbilitySystemComponent* ASC);
	void TimerOff(UAbilitySystemComponent* ASC);
	void EvaluateDamage();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion|Component")
	TObjectPtr<USphereComponent> ExplosionRange;

	TSubclassOf<UGameplayEffect> ExplosionDamageGE;
	TSubclassOf<UGameplayEffect> ContinuousDamageGE;
	TSubclassOf<UGameplayEffect> DestructDamageGE;

	float ExplosionDamage;
	float ContinuousDamage;
	float DestructDamage;

	FTimerHandle DamageTimer;

	FName PDAType;
	FName PDAName;
	EExplosionAreaDamageType Type;
	float Duration;
	TSet<UAbilitySystemComponent*> DamageCandidates;
	TMap<UAbilitySystemComponent*, FActiveGameplayEffectHandle> ActiveEffectHandles;
	TMap<UAbilitySystemComponent*, FTimerHandle> EffectTimers;
	UFXSystemComponent* ContinuousParticle;

private:
	static TSet<FName> PDANameSet;
};
