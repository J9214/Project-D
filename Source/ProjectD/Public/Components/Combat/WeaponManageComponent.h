#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GameplayAbilitySpec.h"
#include "WeaponManageComponent.generated.h"

class APDWeaponBase;
class UDataAsset_Weapon;
class UPDAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedWeaponDataChanged, APDWeaponBase*);
DECLARE_MULTICAST_DELEGATE(FOnWeaponSlotsChanged);

UENUM(BlueprintType)
enum class EWeaponDragSourceType : uint8
{
	Slot,
	External
};

USTRUCT(BlueprintType)
struct FWeaponPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponDragSourceType SourceType = EWeaponDragSourceType::Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FromSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APDWeaponBase> WeaponClass = nullptr;
};

USTRUCT(BlueprintType)
struct FWeaponSlot
{
	GENERATED_BODY()

public:
	FORCEINLINE bool IsEmpty() const { return WeaponActor == nullptr; }
	
public:
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
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Weapon")
	void Server_HandleWeaponEquip(const FWeaponPayload& Payload, int32 ToSlotIndex);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Weapon")
	void Server_RemoveWeaponFromSlot(int32 SlotIndex);
	
	void EquipSlot(int32 SlotIndex);
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
	
	void ApplyBuy(TSubclassOf<APDWeaponBase> WeaponClass);
	void ApplyAssign(int32 ToIndex, TSubclassOf<APDWeaponBase> WeaponClass);
	void ApplyMoveOrSwap(int32 FromIndex, int32 ToIndex);
	void ApplyRemove(int32 SlotIndex, bool bDestroy = true);
	
    APDWeaponBase* SpawnWeaponActor(TSubclassOf<APDWeaponBase> WeaponClass);
	bool SpawnAndPlace(int32 SlotIndex, TSubclassOf<APDWeaponBase> WeaponClass);
	
    void AttachToHand(APDWeaponBase* Weapon);
    void AttachToBack(APDWeaponBase* Weapon, int32 SlotIndex);

    UAbilitySystemComponent* GetASC() const;
    void GrantAbilitiesFromWeaponData(UDataAsset_Weapon* WeaponData);
    void RemoveCurrentWeaponGrantedAbilities();
	
    int32 FindFirstEmptySlot() const;
    int32 FindSlotIndexByWeapon(APDWeaponBase* Weapon) const;

public:
	FOnEquippedWeaponDataChanged OnEquippedWeaponDataChanged;
	FOnWeaponSlotsChanged OnWeaponSlotsChanged;
	
protected:
    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<APDWeaponBase> EquippedWeapon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sockets")
    TArray<FName> BackSocketNames;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sockets")
	FName HandSocketName;
	
private:
    UPROPERTY(ReplicatedUsing = OnRep_Slots)
    TArray<FWeaponSlot> Slots;

    UPROPERTY(ReplicatedUsing = OnRep_EquippedSlotIndex)
    int32 EquippedSlotIndex = INDEX_NONE;

    UPROPERTY()
    TArray<FGameplayAbilitySpecHandle> CurrentWeaponGrantedAbilityHandles;
	
	bool bPendingRefreshAttachments = false;
};
