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

	UFUNCTION(Server, Reliable)
	void Server_SetFireMode(EPDWeaponFireMode NewMode);
	
	FORCEINLINE EPDWeaponFireMode GetCurrentFireMode() const { return CurrentFireMode; }
	FORCEINLINE bool IsFullAuto() const { return CurrentFireMode == EPDWeaponFireMode::FullAuto; }
	
	FORCEINLINE USceneComponent* GetMuzzleComponent() const { return Muzzle; }
	FVector GetMuzzlePoint() const;
	
	bool CanFire() const;
	void ConsumeAmmo(int32 Amount);
	
	void InitWeaponData();
	
	int32 GetMaxAmmo() const { return MaxAmmo; }
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	void ReloadAmmo();

	EPDWeaponFireMode GetNextFireMode() const;
	
	UFUNCTION(BlueprintCallable, Category="Weapon|Aim")
	FTransform GetSightCameraWorldTransform() const;
	
protected:
	UFUNCTION()
	void OnRep_AmmoChanged();
	
	UFUNCTION()
	void OnRep_FireModeChanged();
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDataAsset_Weapon> WeaponData;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> RootComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Muzzle;
	
	UPROPERTY(ReplicatedUsing=OnRep_FireModeChanged, VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	EPDWeaponFireMode CurrentFireMode = EPDWeaponFireMode::SemiAuto;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	int32 MaxAmmo = 0;

	UPROPERTY(ReplicatedUsing=OnRep_AmmoChanged, VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	int32 CurrentAmmo = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Aim")
	FName SightAnchorComponentName = TEXT("SightCameraAnchor");
};
