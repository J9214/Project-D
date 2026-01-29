#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PDWeaponMontages.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EPDWeaponMontageAction : uint8
{
	Equip,
	Unequip,
	Fire,
	Reload
};

USTRUCT(BlueprintType)
struct FPDWeaponMontageEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	FName SectionName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	float PlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	FGameplayTag CommitEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	bool bStopWhenAbilityEnds = true;

public:
	FORCEINLINE bool IsValid() const { return Montage != nullptr; }
};

USTRUCT(BlueprintType)
struct FPDWeaponMontageSet
{
	GENERATED_BODY()

public:
	FORCEINLINE const FPDWeaponMontageEntry& Get(EPDWeaponMontageAction Action) const
	{
		switch (Action)
		{
		case EPDWeaponMontageAction::Equip:  
			return Equip;
		case EPDWeaponMontageAction::Unequip:
			return Unequip;
		case EPDWeaponMontageAction::Fire:  
			return Fire;
		case EPDWeaponMontageAction::Reload: 
			return Reload;
		default:                             
			return Equip;
		}
	}
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	FPDWeaponMontageEntry Equip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	FPDWeaponMontageEntry Unequip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	FPDWeaponMontageEntry Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	FPDWeaponMontageEntry Reload;
};