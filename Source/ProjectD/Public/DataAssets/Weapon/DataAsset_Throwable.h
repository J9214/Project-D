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

UCLASS()
class PROJECTD_API UDataAsset_Throwable : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> ThrowableIMC = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	int32 IMCPriority = 50;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable")
	TSubclassOf<APDThrowableProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable")
	float MinThrowSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable")
	float MaxThrowSpeed = 1800.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable")
	float GravityScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable")
	float FuseTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable")
	float ExplosionRadius = 350.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable")
	TSubclassOf<UGameplayEffect> ExplosionGE;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Preview")
	float PredictSimTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Preview")
	float PredictStep = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Preview")
	TObjectPtr<UNiagaraSystem> TrajectoryNiagara;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equip")
	FPDWeaponMontageEntry EquipEntry;
};
