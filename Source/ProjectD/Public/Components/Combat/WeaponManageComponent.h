#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GameplayAbilitySpec.h"
#include "WeaponManageComponent.generated.h"

class APDWeaponBase;
class UDataAsset_Weapon;
class UPDAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedWeaponDataChanged, APDWeaponBase*);

USTRUCT(BlueprintType)
struct FPTWeaponSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<APDWeaponBase> WeaponActor = nullptr;
};

UCLASS()
class PROJECTD_API UWeaponManageComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:
    UWeaponManageComponent();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_BuyWeapon(TSubclassOf<APDWeaponBase> WeaponClass);
	
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool AddWeaponToInventory(TSubclassOf<APDWeaponBase> WeaponClass);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_RequestMoveOrSwapSlot(int32 FromIndex, int32 ToIndex);
	
	UFUNCTION(BlueprintCallable, Category="Weapon|Inventory")
	void ApplyMoveOrSwapSlot(int32 FromIndex, int32 ToIndex);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void EquipSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void UnequipCurrentWeapon();

    FORCEINLINE APDWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }
	APDWeaponBase* GetWeaponInSlot(int32 SlotIndex) const;

protected:
    virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UFUNCTION()
    void OnRep_Slots();

    UFUNCTION()
    void OnRep_EquippedSlotIndex();
	
    UFUNCTION()
	void OnRep_EquippedWeapon();
	
    UFUNCTION()
	void DoRefreshAttachments();
	
    void RefreshAttachments();
	void ScheduleRefreshAttachments();
	
    int32 FindFirstEmptySlot() const;
    int32 FindSlotIndexByWeapon(APDWeaponBase* Weapon) const;

    APDWeaponBase* SpawnWeaponActor(TSubclassOf<APDWeaponBase> WeaponClass);

    void AttachToHand(APDWeaponBase* Weapon);
    void AttachToBack(APDWeaponBase* Weapon, int32 SlotIndex);

    UAbilitySystemComponent* GetASC() const;
    void GrantAbilitiesFromWeaponData(UDataAsset_Weapon* WeaponData);
    void RemoveCurrentWeaponGrantedAbilities();

public:
	FOnEquippedWeaponDataChanged OnEquippedWeaponDataChanged;
	
protected:
    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<APDWeaponBase> EquippedWeapon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sockets")
    TArray<FName> BackSocketNames;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sockets")
	FName HandSocketName;
	
private:
    UPROPERTY(ReplicatedUsing = OnRep_Slots)
    TArray<FPTWeaponSlot> Slots;

    UPROPERTY(ReplicatedUsing = OnRep_EquippedSlotIndex)
    int32 EquippedSlotIndex = INDEX_NONE;

    UPROPERTY()
    TArray<FGameplayAbilitySpecHandle> CurrentWeaponGrantedAbilityHandles;
	
	bool bPendingRefreshAttachments = false;
};
