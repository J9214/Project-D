// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Shop/FPDItemInfo.h"
#include "PDInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct PROJECTD_API FPDItemData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Count = 0;

	bool IsNone() const { return ItemID.IsNone(); }
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTD_API UPDInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPDInventoryComponent();
	
	bool AddItem(const FPDItemInfo* ItemInfo);
	bool AddItem_ETC(const FPDItemInfo* ItemInfo);
	bool AddItem_Weapon(const FPDItemInfo* ItemInfo);
	bool AddItem_Grenade(const FPDItemInfo* ItemInfo);
	bool AddItem_Skill(const FPDItemInfo* ItemInfo);
	bool ConsumeGrenadeAtSlot(int32 SlotIndex);
	bool HasGrenadeAtSlot(int32 SlotIndex) const;

	void AddGold(int InGold);
	bool CheckGold(int Cost) const { return Gold >= Cost; };

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SwapWeaponItem(int32 FromSlotIndex, int32 SlotIndex, FPDItemData ItemInfo);
	void SwapSkillItem(int32 FromSlotIndex, int32 SlotIndex, FPDItemData ItemInfo);
	void SwapGrenadeItem(int32 FromSlotIndex, int32 SlotIndex, FPDItemData ItemInfo);
	void SwapETCItem(int32 FromSlotIndex, int32 SlotIndex, FPDItemData ItemInfo);

	void ClearInventoryToDefault();
	void ClearCombatInventoryOnDeath();

	UFUNCTION(Server, Reliable)
	void SwapItem(EItemType FromItemType, FName FromItemId, int32 FromSlot, int32 OringinCount, EItemType ItemType, FName ItemId, int32 Slot, int32 Count);

	void UpdateSlotByType(EItemType FromType, EItemType Type, int32 FromSlotIndex, int32 SlotIndex, FPDItemData NewData);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_Gold();

	UFUNCTION()
	void OnRep_WeaponChanged(const TArray<FPDItemData>& OldItemSlot);

	UFUNCTION()
	void OnRep_SkillChanged(const TArray<FPDItemData>& OldItemSlot);

	UFUNCTION()
	void OnRep_GrenadeChanged(const TArray<FPDItemData>& OldItemSlot);

	UFUNCTION()
	void OnRep_ETCChanged(const TArray<FPDItemData>& OldItemSlot);

	UFUNCTION()
	void ItemChanged(EItemType ItemType, const TArray<FPDItemData>& OldItemSlot);

	TArray<FPDItemData>& GetItemSlot(EItemType ItemType);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Gold, BlueprintReadOnly)
	int Gold = 10;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponChanged, BlueprintReadOnly)
	TArray<FPDItemData> WeaponSlot;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SkillChanged, BlueprintReadOnly)
	TArray<FPDItemData> SkillSlot;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_GrenadeChanged, BlueprintReadOnly)
	TArray<FPDItemData> GrenadeSlot;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ETCChanged, BlueprintReadOnly)
	TArray<FPDItemData> ETCSlot;
};
