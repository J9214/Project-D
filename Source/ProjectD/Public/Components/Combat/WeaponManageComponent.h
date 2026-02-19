#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GameplayAbilitySpec.h"
#include "Weapon/PDWeaponMontages.h"
#include "WeaponManageComponent.generated.h"

class APDWeaponBase;
class APDThrowableItemBase;
class UDataAsset_Weapon;
class UPDAbilitySystemComponent;
class UEnhancedInputLocalPlayerSubsystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedWeaponChanged, APDWeaponBase*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedThrowableChanged, APDThrowableItemBase*);
DECLARE_MULTICAST_DELEGATE(FOnWeaponSlotsChanged);

UENUM(BlueprintType)
enum class EEquipmentCategory : uint8
{
	Weapon,
	Throwable
};

UENUM(BlueprintType)
enum class EWeaponDragSourceType : uint8
{
	Slot,
	External
};

USTRUCT(BlueprintType)
struct FEquipmentPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEquipmentCategory Category = EEquipmentCategory::Weapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponDragSourceType SourceType = EWeaponDragSourceType::Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FromSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APDWeaponBase> WeaponClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APDThrowableItemBase> ThrowableItemClass = nullptr;
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

USTRUCT(BlueprintType)
struct FThrowableSlot
{
	GENERATED_BODY()

public:
	FORCEINLINE bool IsEmpty() const { return ThrowableItemActor == nullptr; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<APDThrowableItemBase> ThrowableItemActor = nullptr;
};

UCLASS()
class PROJECTD_API UWeaponManageComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:
    UWeaponManageComponent();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_BuyWeapon(TSubclassOf<APDWeaponBase> WeaponClass);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_BuyThrowable(TSubclassOf<APDThrowableItemBase> ThrowableItemClass);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Weapon")
	void Server_HandleWeaponEquip(const FEquipmentPayload& Payload, int32 ToSlotIndex);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Weapon")
	void Server_RemoveWeaponFromSlot(int32 SlotIndex);
	
	void EquipSlot(int32 SlotIndex);
	void UnequipCurrentWeapon();
	
    FORCEINLINE APDWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE APDThrowableItemBase* GetEquippedThrowable() const { return EquippedThrowable; }
	APDWeaponBase* GetWeaponInSlot(int32 SlotIndex) const;
	FORCEINLINE int32 GetEquippedSlotIndex() const { return EquippedSlotIndex; }

	bool TryGetEquipEntry(int32 SlotIndex, FPDWeaponMontageEntry& OutEntry) const;
	bool HasItemInSlot(int32 SlotIndex) const;
	
	UFUNCTION(BlueprintCallable, Category="Input")
	UInputMappingContext* GetEquippedIMC(int32& OutPriority) const;
	
protected:
    virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UFUNCTION()
    void OnRep_Slots();

	UFUNCTION()
	void OnRep_ThrowableSlots();
	
    UFUNCTION()
	void OnRep_EquippedWeapon();
	
	UFUNCTION()
	void OnRep_EquippedThrowable();
	
    UFUNCTION()
    void OnRep_EquippedSlotIndex();
	
    UFUNCTION()
	void DoRefreshAttachments();
	
    void RefreshAttachments();
	void ScheduleRefreshAttachments();
	
	void ApplyBuy_Weapon(TSubclassOf<APDWeaponBase> WeaponClass);
	void ApplyBuy_Throwable(TSubclassOf<APDThrowableItemBase> ThrowableItemClass);
	
	void ApplyAssign(int32 ToIndex, TSubclassOf<APDWeaponBase> WeaponClass);
	void ApplyAssign_Throwable(int32 ToIndex, TSubclassOf<APDThrowableItemBase> ThrowableItemClass);
	
	void ApplyMoveOrSwap_Global(int32 FromIndex, int32 ToIndex);
	void ApplyMoveOrSwap(int32 FromIndex, int32 ToIndex);
	void ApplyMoveOrSwap_Throwable(int32 FromIndex, int32 ToIndex);
	
	void ApplyRemove_Global(int32 SlotIndex, bool bDestroy = true);
	void ApplyRemove(int32 SlotIndex, bool bDestroy = true);
	void ApplyRemove_Throwable(int32 SlotIndex, bool bDestroy = true);
	
    APDWeaponBase* SpawnWeaponActor(TSubclassOf<APDWeaponBase> WeaponClass);
	APDThrowableItemBase* SpawnThrowableItemActor(TSubclassOf<APDThrowableItemBase> ThrowableItemClass);
	bool SpawnAndPlace(int32 SlotIndex, TSubclassOf<APDWeaponBase> WeaponClass);
	bool SpawnAndPlace_Throwable(int32 SlotIndex, TSubclassOf<APDThrowableItemBase> ThrowableItemClass);
	
    void AttachToHand(APDWeaponBase* Weapon);
    void AttachToBack(APDWeaponBase* Weapon, int32 SlotIndex);
	void ThrowableAttachToHand(APDThrowableItemBase* Throwable);
	void ThrowableAttachToBack(APDThrowableItemBase* Throwable, int32 SlotIndex);

    UAbilitySystemComponent* GetASC() const;
    void GrantAbilitiesFromWeaponData(UDataAsset_Weapon* WeaponData);
    void RemoveCurrentWeaponGrantedAbilities();
	
    int32 FindFirstEmptySlot() const;
    int32 FindFirstEmptyThrowableSlot() const;
    int32 FindSlotIndexByWeapon(APDWeaponBase* Weapon) const;

	FORCEINLINE bool IsWeaponSlotIndex(int32 SlotIndex) const { return SlotIndex >= 0 && SlotIndex < WeaponSlotCount; }
	FORCEINLINE bool IsThrowableSlotIndex(int32 SlotIndex) const { return SlotIndex >= WeaponSlotCount && SlotIndex < TotalSlotCount; }
	FORCEINLINE int32 ToThrowableLocalIndex(int32 SlotIndex) const { return SlotIndex - WeaponSlotCount; }
	
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(APawn* Pawn);
	void RefreshEquipIMC();
	
public:
	FOnEquippedWeaponChanged OnEquippedWeaponChanged;
	FOnEquippedThrowableChanged OnEquippedThrowableChanged;
	FOnWeaponSlotsChanged OnWeaponSlotsChanged;
	
protected:
    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<APDWeaponBase> EquippedWeapon = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_EquippedThrowable, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<APDThrowableItemBase> EquippedThrowable = nullptr;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sockets")
    TArray<FName> BackSocketNames;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sockets")
	TArray<FName> ThrowableBackSocketNames;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sockets")
	FName HandSocketName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sockets")
	FName ThrowableHandSocketName;
	
private:
    UPROPERTY(ReplicatedUsing = OnRep_Slots)
    TArray<FWeaponSlot> Slots;
	
	UPROPERTY(ReplicatedUsing=OnRep_ThrowableSlots)
	TArray<FThrowableSlot> ThrowableSlots;

    UPROPERTY(ReplicatedUsing = OnRep_EquippedSlotIndex)
    int32 EquippedSlotIndex = INDEX_NONE;

    UPROPERTY()
    TArray<FGameplayAbilitySpecHandle> CurrentWeaponGrantedAbilityHandles;
	
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> CurrentThrowableGrantedAbilityHandles;
	
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> CurrentAppliedIMC = nullptr;
	
	UPROPERTY(Transient)
	int32 CurrentAppliedIMCPriority = 0;
	
	bool bPendingRefreshAttachments = false;
	
	int32 WeaponSlotCount;
	int32 ThrowableSlotCount;
	int32 TotalSlotCount;
};
