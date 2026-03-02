// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/PDInventoryComponent.h"
#include "Components/Shop/FPDItemInfo.h"
#include "Net/UnrealNetwork.h"
#include "Controller/PDPlayerController.h"
#include "Pawn/PDPawnBase.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "GameInstance/Subsystem/PDItemInfoSubsystem.h"
#include "Weapon/PDWeaponBase.h"
#include "PlayerState/PDPlayerState.h"

// Sets default values for this component's properties
UPDInventoryComponent::UPDInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Gold = 10;

	WeaponSlot.Init(FPDItemData(), 2);
	SkillSlot.Init(FPDItemData(), 2);
	GrenadeSlot.Init(FPDItemData(), 2);
	ETCSlot.Init(FPDItemData(), 8);
}

bool UPDInventoryComponent::AddItem(const FPDItemInfo* ItemInfo)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return false;
	}
	switch (ItemInfo->ItemType)
	{
	case EItemType::Weapon:
		return AddItem_Weapon(ItemInfo);
	case EItemType::Skill:
		return AddItem_Skill(ItemInfo);
	case EItemType::Grenade:
		return AddItem_Grenade(ItemInfo);
	case EItemType::Etc:
		return AddItem_ETC(ItemInfo);
	default:
		//에러 로그
		return false;
	}
}

bool UPDInventoryComponent::AddItem_Weapon(const FPDItemInfo* ItemInfo)
{
	int32 TargetIndex = INDEX_NONE;
	auto IsEmpty = [](const FPDItemData& Data) { return Data.ItemID.IsNone(); };

	TargetIndex = WeaponSlot.IndexOfByPredicate(IsEmpty);

	if (TargetIndex == INDEX_NONE)
	{
		TargetIndex = WeaponSlot.IndexOfByPredicate([](const FPDItemData& Data)
			{
				return Data.ItemID == FName("DefaultWeapon");
			});
	}

	if (TargetIndex != INDEX_NONE)
	{
		APDPlayerState* PS = Cast<APDPlayerState>(GetOwner());
		if (!PS)
		{
			UE_LOG(LogTemp, Error, TEXT("0"));
			return false;
		}

		APlayerController* PC = Cast<APlayerController>(PS->GetPlayerController());
		if (!PC)
		{
			UE_LOG(LogTemp, Warning, TEXT("00"));
			return false;
		}

		APDPawnBase* OwnerPawn = Cast<APDPawnBase>(PC->GetPawn());
		if (!OwnerPawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("01"));
			return false;
		}

		UWeaponManageComponent* WMC = OwnerPawn->GetWeaponManageComponent();
		if (!WMC)
		{
			UE_LOG(LogTemp, Warning, TEXT("02"));
			return false;
		}

		TSubclassOf<APDWeaponBase> SelectedWeaponClass = *(ItemInfo->ItemClass);
		if (!SelectedWeaponClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("03"));
			return false;
		}

		FEquipmentPayload InitPayload;
		InitPayload.Category = EEquipmentCategory::Weapon;
		InitPayload.SourceType = EWeaponDragSourceType::External;
		InitPayload.FromSlotIndex = TargetIndex;
		InitPayload.WeaponClass = SelectedWeaponClass;
		InitPayload.ThrowableItemClass = nullptr;

		WeaponSlot[TargetIndex].ItemID = ItemInfo->ItemID;
		WMC->Server_HandleWeaponEquip(InitPayload, TargetIndex);

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("04"));
		return false;
	}

	return true;
}

bool UPDInventoryComponent::AddItem_Grenade(const FPDItemInfo* ItemInfo)
{
	int32 TargetIndex = INDEX_NONE;
	auto IsEmpty = [](const FPDItemData& Data) { return Data.ItemID.IsNone(); };

	TargetIndex = GrenadeSlot.IndexOfByPredicate([&ItemInfo](const FPDItemData& Data)
		{ return Data.ItemID == ItemInfo->ItemID; });

	if (TargetIndex != INDEX_NONE)
	{
		GrenadeSlot[TargetIndex].Count++;
	}
	else
	{
		TargetIndex = GrenadeSlot.IndexOfByPredicate(IsEmpty);

		if (TargetIndex != INDEX_NONE)
		{
			APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
			if (!OwnerPawn)
			{
				return false;
			}

			UWeaponManageComponent* WMC = OwnerPawn->GetWeaponManageComponent();
			if (!WMC)
			{
				return false;
			}

			TSubclassOf<APDWeaponBase> ThrowableItemClass = *(ItemInfo->ItemClass);
			if (!ThrowableItemClass)
			{
				return false;
			}

			FEquipmentPayload InitPayload;
			InitPayload.Category = EEquipmentCategory::Weapon;
			InitPayload.SourceType = EWeaponDragSourceType::External;
			InitPayload.FromSlotIndex = TargetIndex + 2;
			InitPayload.ThrowableItemClass = ThrowableItemClass;
			InitPayload.WeaponClass = nullptr;

			GrenadeSlot[TargetIndex].ItemID = ItemInfo->ItemID;
			GrenadeSlot[TargetIndex].Count = 1;

			WMC->Server_HandleWeaponEquip(InitPayload, TargetIndex + 2);
		}
		else
		{
			return AddItem_ETC(ItemInfo);
		}
	}

	return true;
}

bool UPDInventoryComponent::AddItem_Skill(const FPDItemInfo* ItemInfo)
{
	int32 TargetIndex = INDEX_NONE;
	auto IsEmpty = [](const FPDItemData& Data) { return Data.ItemID.IsNone(); };

	TargetIndex = SkillSlot.IndexOfByPredicate(IsEmpty);

	if (TargetIndex != INDEX_NONE)
	{
		SkillSlot[TargetIndex].ItemID = ItemInfo->ItemID;
	}
	else
	{
		return AddItem_ETC(ItemInfo);
	}

	return true;
}
bool UPDInventoryComponent::AddItem_ETC(const FPDItemInfo* ItemInfo)
{
	auto IsEmpty = [](const FPDItemData& Data) { return Data.ItemID.IsNone(); };

	for (auto& item : ETCSlot)
	{
		if (item.ItemID == ItemInfo->ItemID)
		{
			item.Count++;
			return true;
		}
	}
	int TargetIndex = ETCSlot.IndexOfByPredicate(IsEmpty);

	if (TargetIndex != INDEX_NONE)
	{
		ETCSlot[TargetIndex].ItemID = ItemInfo->ItemID;
		ETCSlot[TargetIndex].Count = 1;

		return true;
	}

	return false;
}

void UPDInventoryComponent::AddGold(int InGold)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	Gold = FMath::Max(0, Gold + InGold);
}

void UPDInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPDInventoryComponent, Gold);
	DOREPLIFETIME(UPDInventoryComponent, WeaponSlot);
	DOREPLIFETIME(UPDInventoryComponent, SkillSlot);
	DOREPLIFETIME(UPDInventoryComponent, GrenadeSlot);
	DOREPLIFETIME(UPDInventoryComponent, ETCSlot);
}

void UPDInventoryComponent::SwapWeaponItem(int SlotIndex, FPDItemData ItemData)
{
	if (GetOwnerRole() < ROLE_Authority || !WeaponSlot.IsValidIndex(SlotIndex))
	{
		return;
	}

	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	UWeaponManageComponent* WMC = OwnerPawn->GetWeaponManageComponent();
	if (!WMC)
	{
		return;
	}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		return;
	}

	UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
	if (!ItemSubsystem)
	{
		return;
	}

	const FPDItemInfo* Info = ItemSubsystem->GetItemInfoByName(ItemData.ItemID);
	if (!Info || !Info->ItemClass)
	{
		return;
	}

	FEquipmentPayload Payload;
	Payload.SourceType = EWeaponDragSourceType::External;
	Payload.Category = EEquipmentCategory::Weapon;

	if (Info && Info->ItemClass)
	{
		Payload.WeaponClass = *Info->ItemClass;
		WMC->Server_HandleWeaponEquip(Payload, SlotIndex);
	}

	//TSubclassOf<APDWeaponBase> NewWeaponClass = *Info->ItemClass;

	//WeaponSlot[SlotIndex] = ItemData;
	//WMC->ChangeWeapon(SlotIndex, NewWeaponClass);
}

void UPDInventoryComponent::SwapSkillItem(int SlotIndex, FPDItemData ItemInfo)
{

	if (GetOwnerRole() < ROLE_Authority || !SkillSlot.IsValidIndex(SlotIndex))
	{
		return;
	}

	SkillSlot[SlotIndex] = ItemInfo;
}

void UPDInventoryComponent::SwapGrenadeItem(int SlotIndex, FPDItemData ItemInfo)
{

	if (GetOwnerRole() < ROLE_Authority || !GrenadeSlot.IsValidIndex(SlotIndex))
	{
		return;
	}

	GrenadeSlot[SlotIndex] = ItemInfo;
}

void UPDInventoryComponent::SwapETCItem(int SlotIndex, FPDItemData ItemInfo)
{
	if (GetOwnerRole() < ROLE_Authority || !ETCSlot.IsValidIndex(SlotIndex))
	{
		return;
	}

	ETCSlot[SlotIndex] = ItemInfo;
}

void UPDInventoryComponent::ClearInventoryToDefault()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return;
	}

	UWeaponManageComponent* WMC = OwnerPawn->GetWeaponManageComponent();
	if (!WMC)
	{
		return;
	}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		return;
	}

	UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
	if (!ItemSubsystem)
	{
		return;
	}

	const FName DefaultWeaponID = FName("DefaultWeapon");
	const FPDItemInfo* DefaultInfo = ItemSubsystem->GetItemInfoByName(DefaultWeaponID);

	TSubclassOf<APDWeaponBase> DefaultClass = (DefaultInfo && DefaultInfo->ItemClass) ? *DefaultInfo->ItemClass : nullptr;

	WeaponSlot.Init(FPDItemData(), WeaponSlot.Num());
	SkillSlot.Init(FPDItemData(), SkillSlot.Num());
	GrenadeSlot.Init(FPDItemData(), GrenadeSlot.Num());
	ETCSlot.Init(FPDItemData(), ETCSlot.Num());

	if (DefaultClass)
	{
		WeaponSlot[0].ItemID = DefaultWeaponID;

		WMC->ChangeWeapon(0, DefaultClass);
	}

	for (int32 i = 1; i < WeaponSlot.Num(); ++i)
	{
		WMC->ChangeWeapon(i, nullptr);
	}
}

void UPDInventoryComponent::OnRep_Gold()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APDPlayerController* PC = Cast<APDPlayerController>(OwnerPawn->GetController());
	if (PC)
	{
		PC->InitGoldDisplay(Gold);
	}
}

void UPDInventoryComponent::OnRep_WeaponChanged(const TArray<FPDItemData>& OldItemSlot)
{
	ItemChanged(EItemType::Weapon,OldItemSlot);
}

void UPDInventoryComponent::OnRep_SkillChanged(const TArray<FPDItemData>& OldItemSlot)
{
	ItemChanged(EItemType::Skill, OldItemSlot);
}

void UPDInventoryComponent::OnRep_GrenadeChanged(const TArray<FPDItemData>& OldItemSlot)
{
	ItemChanged(EItemType::Grenade, OldItemSlot);
}

void UPDInventoryComponent::OnRep_ETCChanged(const TArray<FPDItemData>& OldItemSlot)
{
	ItemChanged(EItemType::Etc, OldItemSlot);
}
void UPDInventoryComponent::ItemChanged(EItemType ItemType, const TArray<FPDItemData>& OldItemSlot)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APDPlayerController* PC = Cast<APDPlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		return;
	}

	const TArray<FPDItemData>& CurrentSlot = GetItemSlot(ItemType);
	int32 MaxIndex = CurrentSlot.Num();
	for (int32 i = 0; i < MaxIndex; ++i)
	{
		bool bIsSame = false;
		if (OldItemSlot.IsValidIndex(i))
		{
			if (CurrentSlot[i].ItemID == OldItemSlot[i].ItemID &&
				CurrentSlot[i].Count == OldItemSlot[i].Count)
			{
				bIsSame = true;
			}
		}

		if (!bIsSame)
		{
			PC->InitItemDataDisplay(ItemType, i, CurrentSlot[i].ItemID, CurrentSlot[i].Count);
		}
	}
}

TArray<FPDItemData>& UPDInventoryComponent::GetItemSlot(EItemType ItemType)
{
	switch (ItemType)
	{
	case EItemType::Weapon:  return WeaponSlot;
	case EItemType::Skill:   return SkillSlot;
	case EItemType::Grenade: return GrenadeSlot;
	default:                 return ETCSlot;
	}
}

// Called when the game starts
void UPDInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

