// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/PDInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Controller/PDPlayerController.h"
#include "Pawn/PDPawnBase.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "GameInstance/Subsystem/PDItemInfoSubsystem.h"
#include "Weapon/PDWeaponBase.h"
#include "Weapon/PDThrowableItemBase.h"
#include "PlayerState/PDPlayerState.h"

// Sets default values for this component's properties
UPDInventoryComponent::UPDInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	Gold = 10;

	WeaponSlot.Init(FPDItemData(), 2);
	SkillSlot.Init(FPDItemData(), 2);
	GrenadeSlot.Init(FPDItemData(), 2);
	ETCSlot.Init(FPDItemData(), 8);
}

bool UPDInventoryComponent::AddItem(const FPDItemInfo* ItemInfo)
{
	//if (GetOwnerRole() < ROLE_Authority)
	//{
	//	return false;
	//}
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
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem ItemType default"));
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

	UE_LOG(LogTemp, Error, TEXT("TargetIndex : %d"), TargetIndex);

	if (TargetIndex != INDEX_NONE)
	{
		APDPlayerState* PS = Cast<APDPlayerState>(GetOwner());
		if (!PS)
		{
			UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon PS Null"));
			return false;
		}

		APlayerController* PC = Cast<APlayerController>(PS->GetPlayerController());
		if (!PC)
		{
			UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon PC Null"));
			return false;
		}

		APDPawnBase* OwnerPawn = Cast<APDPawnBase>(PC->GetPawn());
		if (!OwnerPawn)
		{
			UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon OwnerPawn Null"));
			return false;
		}

		UWeaponManageComponent* WMC = OwnerPawn->GetWeaponManageComponent();
		if (!WMC)
		{
			UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon WMC Null"));
			return false;
		}

		TSubclassOf<APDWeaponBase> SelectedWeaponClass = *(ItemInfo->ItemClass);
		if (!SelectedWeaponClass)
		{
			UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon SelectedWeaponClass Null"));
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
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon TargetIndex == INDEX_NONE"));
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
			APDPlayerState* PS = Cast<APDPlayerState>(GetOwner());
			if (!PS)
			{
				UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon PS Null"));
				return false;
			}

			APlayerController* PC = Cast<APlayerController>(PS->GetPlayerController());
			if (!PC)
			{
				UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon PC Null"));
				return false;
			}

			APDPawnBase* OwnerPawn = Cast<APDPawnBase>(PC->GetPawn());
			if (!OwnerPawn)
			{
				UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Weapon OwnerPawn Null"));
				return false;
			}

			UWeaponManageComponent* WMC = OwnerPawn->GetWeaponManageComponent();
			if (!WMC)
			{
				UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Grenade WMC Null"));
				return false;
			}

			TSubclassOf<APDThrowableItemBase> ThrowableItemClass = *(ItemInfo->ItemClass);
			if (!ThrowableItemClass)
			{
				UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::AddItem_Grenade ThrowableItemClass Null"));
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

	UE_LOG(LogTemp, Warning, TEXT("UPDInventoryComponent::AddItem_ETC AddItem_ETC Fail"));
	return false;
}

void UPDInventoryComponent::AddGold(int InGold)
{
	//if (GetOwnerRole() < ROLE_Authority)
	//{
	//	return;
	//}

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
	//if (GetOwnerRole() < ROLE_Authority || !WeaponSlot.IsValidIndex(SlotIndex))
	//{
	//	return;
	//}

	APDPlayerState* PS = Cast<APDPlayerState>(GetOwner());
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::SwapWeaponItem PS Null"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(PS->GetPlayerController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::SwapWeaponItem PC Null"));
		return;
	}

	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(PC->GetPawn());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::SwapWeaponItem OwnerPawn Null"));
		return;
	}

	UWeaponManageComponent* WMC = OwnerPawn->GetWeaponManageComponent();
	if (!WMC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::SwapWeaponItem WMC Null"));
		return;
	}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::SwapWeaponItem GI Null"));
		return;
	}

	UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
	if (!ItemSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::SwapWeaponItem ItemSubsystem Null"));
		return;
	}

	WeaponSlot[SlotIndex] = ItemData;

	const FPDItemInfo* Info = ItemSubsystem->GetItemInfoByName(ItemData.ItemID);
	if (!Info || !Info->ItemClass)
	{
		WMC->Server_RemoveWeaponFromSlot(SlotIndex);
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
}

void UPDInventoryComponent::SwapSkillItem(int SlotIndex, FPDItemData ItemInfo)
{

	//if (GetOwnerRole() < ROLE_Authority || !SkillSlot.IsValidIndex(SlotIndex))
	//{
	//	return;
	//}

	SkillSlot[SlotIndex] = ItemInfo;
}

void UPDInventoryComponent::SwapGrenadeItem(int SlotIndex, FPDItemData ItemInfo)
{

	//if (GetOwnerRole() < ROLE_Authority || !GrenadeSlot.IsValidIndex(SlotIndex))
	//{
	//	return;
	//}

	GrenadeSlot[SlotIndex] = ItemInfo;
}

void UPDInventoryComponent::SwapETCItem(int SlotIndex, FPDItemData ItemInfo)
{
	//if (GetOwnerRole() < ROLE_Authority || !ETCSlot.IsValidIndex(SlotIndex))
	//{
	//	return;
	//}

	ETCSlot[SlotIndex] = ItemInfo;
}

void UPDInventoryComponent::ClearInventoryToDefault()
{
	//if (GetOwnerRole() < ROLE_Authority)
	//{
	//	return;
	//}

	APDPlayerState* PS = Cast<APDPlayerState>(GetOwner());
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::ClearInventoryToDefault PS Null"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(PS->GetPlayerController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::ClearInventoryToDefault PC Null"));
		return;
	}

	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(PC->GetPawn());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::ClearInventoryToDefault OwnerPawn Null"));
		return;
	}

	UWeaponManageComponent* WMC = OwnerPawn->GetWeaponManageComponent();
	if (!WMC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::ClearInventoryToDefault WMC Null"));
		return;
	}

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::ClearInventoryToDefault GI Null"));
		return;
	}

	UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
	if (!ItemSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::ClearInventoryToDefault ItemSubsystem Null"));
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

void UPDInventoryComponent::SwapItem_Implementation(EItemType OriginItemType, FName OriginItemId, int32 OriginSlot, int32 OringinCount, EItemType ItemType, FName ItemId, int32 Slot, int32 Count)
{
	bool bIsSameType = (OriginItemType == ItemType);
	bool bIsGrenadeEtcSwap = (OriginItemType == EItemType::Grenade && ItemType == EItemType::Etc) ||
		(OriginItemType == EItemType::Etc && ItemType == EItemType::Grenade);
	if (!bIsSameType && !bIsGrenadeEtcSwap)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPDInventoryComponent::SwapItem_Implementation SwapType"));
		return;
	}

	TArray<FPDItemData>& OriginArray = GetItemSlot(OriginItemType);
	TArray<FPDItemData>& TargetArray = GetItemSlot(ItemType);

	if (!OriginArray.IsValidIndex(OriginSlot) || !TargetArray.IsValidIndex(Slot))
	{
		UE_LOG(LogTemp, Warning, TEXT("UPDInventoryComponent::SwapItem_Implementation ValidIndex"));
		return;
	}

	if (OriginArray[OriginSlot].ItemID != OriginItemId || TargetArray[Slot].ItemID != ItemId)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPDInventoryComponent::SwapItem_Implementation Data mismatch."));
		return;
	}

	FPDItemData OriginData = OriginArray[OriginSlot];
	FPDItemData TargetData = TargetArray[Slot];

	UpdateSlotByType(OriginItemType, OriginSlot, TargetData);
	UpdateSlotByType(ItemType, Slot, OriginData);
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
	APDPlayerState* PS = Cast<APDPlayerState>(GetOwner());
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::ItemChanged PS Null"));
		return;
	}

	APDPlayerController* PC = Cast<APDPlayerController>(PS->GetPlayerController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDInventoryComponent::ItemChanged PC Null"));
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

void UPDInventoryComponent::UpdateSlotByType(EItemType Type, int32 SlotIndex, FPDItemData NewData)
{
	switch (Type)
	{
	case EItemType::Weapon:
		SwapWeaponItem(SlotIndex, NewData);
		break;
	case EItemType::Grenade:
		SwapGrenadeItem(SlotIndex, NewData);
		break;
	case EItemType::Etc:
		SwapETCItem(SlotIndex, NewData);
		break;
	case EItemType::Skill:
		SwapSkillItem(SlotIndex, NewData);
		break;
	}
}

// Called when the game starts
void UPDInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

