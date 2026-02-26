// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PDInventoryComponent.generated.h"

struct FPDItemInfo;

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

	void AddGold(int InGold);
	bool CheckGold(int Cost) const { return Gold >= Cost; };

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SwapWeaponItem(int SlotIndex, FPDItemData ItemInfo);
	void SwapSkillItem(int SlotIndex, FPDItemData ItemInfo);
	void SwapGrenadeItem(int SlotIndex, FPDItemData ItemInfo);
	void SwapETCItem(int SlotIndex, FPDItemData ItemInfo);

	void ClearInventoryToDefault();


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_Gold();

	UFUNCTION()
	void OnRep_ItemChanged();

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Gold, BlueprintReadOnly)
	int Gold;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ItemChanged, BlueprintReadOnly)
	TArray<FPDItemData> WeaponSlot;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ItemChanged, BlueprintReadOnly)
	TArray<FPDItemData> SkillSlot;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ItemChanged, BlueprintReadOnly)
	TArray<FPDItemData> GrenadeSlot;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ItemChanged, BlueprintReadOnly)
	TArray<FPDItemData> ETCSlot;
};
