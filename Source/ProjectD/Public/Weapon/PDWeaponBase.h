#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/PDWeaponTypes.h"
#include "PDWeaponBase.generated.h"

class UDataAsset_Weapon;

UCLASS()
class PROJECTD_API APDWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APDWeaponBase();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE EPDWeaponFireMode GetCurrentFireMode() const { return CurrentFireMode; }
	FORCEINLINE bool IsFullAuto() const { return CurrentFireMode == EPDWeaponFireMode::FullAuto; }
	
	FVector GetMuzzlePoint() const;
	
	bool ClientCanFire() const;
	bool ServerCanFire() const;
	void ServerConsumeAmmo(int32 Amount);
	
	void InitWeaponData();
	void ChangeFireMode();
	
	int32 GetMaxAmmo() const { return MaxAmmo; }
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	void ReloadAmmo();

protected:
	UFUNCTION()
	void OnRep_AmmoChanged();
	
	EPDWeaponFireMode GetNextFireMode() const;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDataAsset_Weapon> WeaponData;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> RootComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Muzzle;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	EPDWeaponFireMode CurrentFireMode = EPDWeaponFireMode::SemiAuto;
	
	UPROPERTY(ReplicatedUsing=OnRep_AmmoChanged, VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	int32 MaxAmmo = 0;

	UPROPERTY(ReplicatedUsing=OnRep_AmmoChanged, VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	int32 CurrentAmmo = 0;
};
