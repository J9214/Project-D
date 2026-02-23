#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Structs/PDPlayerAbilitySet.h"
#include "Weapon/PDWeaponTypes.h"
#include "Weapon/PDWeaponMontages.h"
#include "GameplayTagContainer.h"
#include "DataAsset_Weapon.generated.h"

class UGameplayEffect;
class UUserWidget;
class USoundBase;
class UNiagaraSystem;
class UInputMappingContext;

UCLASS()
class PROJECTD_API UDataAsset_Weapon : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> WeaponIMC = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	int32 IMCPriority = 50;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EPDWeaponTypes WeaponType = EPDWeaponTypes::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EPDWeaponFireMode DefaultFireMode = EPDWeaponFireMode::SemiAuto;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<EPDWeaponFireMode> SupportedFireModes = { EPDWeaponFireMode::SemiAuto };
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
	TArray<FPDPlayerAbilitySet> GrantedAbilitySets = {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> WeaponDamageGE = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DestructDamageGE = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> FireCooldownGE = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	FPDWeaponMontageSet WeaponMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float WeaponDamage = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DestructDamage = 2.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FireInterval = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxAmmo = 30;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxRange = 10000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Crosshair")
	TSubclassOf<UUserWidget> AimCrosshairWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SFX")
	TObjectPtr<USoundBase> FireSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="VFX")
	TObjectPtr<UNiagaraSystem> MuzzleFlashFX = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Cue")
	FGameplayTag FireCueTag;
};
