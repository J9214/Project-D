#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapon/PDWeaponTypes.h"
#include "Weapon/PDWeaponMontages.h"
#include "GameplayTagContainer.h"
#include "DataAsset_Throwable.generated.h"

class UGameplayEffect;
class UNiagaraSystem;
class UInputMappingContext;
class APDThrowableProjectile;
class APDThrowableFireArea;
class APDThrowableSmokeArea;

UENUM(BlueprintType)
enum class EPDThrowableEffectType : uint8
{
	Fragment,
	Flame,
	Smoke,
	Flash
};

UCLASS()
class PROJECTD_API UDataAsset_Throwable : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> ThrowableIMC = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	int32 IMCPriority = 50;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Type")
	EPDThrowableEffectType EffectType = EPDThrowableEffectType::Fragment;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Projectile")
	TSubclassOf<APDThrowableProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Data")
	float MinThrowSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Data")
	float MaxThrowSpeed = 1800.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Data")
	float GravityScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Data")
	float FuseTime = 3.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Data")
	bool bExplosionOnImpact = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Preview")
	float PredictSimTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Preview")
	float PredictStep = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Preview")
	TObjectPtr<UNiagaraSystem> TrajectoryNiagara;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	FPDWeaponMontageSet ThrowableMontages;
	
	// GameplayCue Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CueTag")
	FGameplayTag ExplosionCueTag;
	
	// Fragment
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Fragment")
	float ExplosionDamage = 100.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Fragment")
	float ExplosionRadius = 350.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Fragment")
	TSubclassOf<UGameplayEffect> ExplosionGE;
	
	// Flame
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Flame")
	float FireDuration = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Flame")
	float FireRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Flame")
	float FireTickInterval = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Flame")
	float FireDamagePerTick = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Flame")
	TSubclassOf<UGameplayEffect> FireDamageGE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Flame")
	TSubclassOf<APDThrowableFireArea> FireAreaClass;
	
	// Smoke
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Smoke")
	float SmokeDuration = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Smoke")
	float SmokeRadius = 450.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Smoke")
	TSubclassOf<UGameplayEffect> SmokeGE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable|Smoke")
	TSubclassOf<APDThrowableSmokeArea> SmokeAreaClass;
};
