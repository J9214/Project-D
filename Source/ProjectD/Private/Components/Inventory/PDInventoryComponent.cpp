// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/PDInventoryComponent.h"
#include "Components/Shop/FPDItemInfo.h"
#include "Net/UnrealNetwork.h"
#include <Controller/PDPlayerController.h>
#include <Pawn/PDPawnBase.h>
#include "Components/Combat/WeaponManageComponent.h"
#include <GameInstance/Subsystem/PDItemInfoSubsystem.h>
#include "Weapon/PDWeaponBase.h"

// Sets default values for this component's properties
UPDInventoryComponent::UPDInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Gold = 0;

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

	int32 TargetIndex = INDEX_NONE;

	auto IsEmpty = [](const FPDItemData& Data) { return Data.ItemID.IsNone(); };

	switch (ItemInfo->ItemType)
	{
	case EItemType::Weapon:
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

			TSubclassOf<APDWeaponBase> WeaponClass = ItemInfo->ItemClass->GetAuthoritativeClass();
			if (!WeaponClass)
			{
				return false;
			}

			WeaponSlot[TargetIndex].ItemID = ItemInfo->ItemID; 
			WMC->ChangeWeapon(TargetIndex, WeaponClass);
		}
		else
		{
			return false;
		}
		break;
	case EItemType::Skill:
		TargetIndex = SkillSlot.IndexOfByPredicate(IsEmpty);

		if (TargetIndex != INDEX_NONE)
		{
			SkillSlot[TargetIndex].ItemID = ItemInfo->ItemID;
		}
		else
		{
			return AddItem_ETC(ItemInfo);
		}
		break;
	case EItemType::Grenade:
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
				GrenadeSlot[TargetIndex].ItemID = ItemInfo->ItemID;
				GrenadeSlot[TargetIndex].Count = 1;
			}
			else
			{
				return AddItem_ETC(ItemInfo);
			}
		}

		break;
	case EItemType::Etc:
		return AddItem_ETC(ItemInfo);
		break;
	default:
		//에러 로그
		break;
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
		GrenadeSlot[TargetIndex].ItemID = ItemInfo->ItemID;
		GrenadeSlot[TargetIndex].Count = 1;

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

	TSubclassOf<APDWeaponBase> NewWeaponClass = *Info->ItemClass;

	WeaponSlot[SlotIndex] = ItemData;
	WMC->ChangeWeapon(SlotIndex, NewWeaponClass);
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

	OnRep_ItemChanged();
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

void UPDInventoryComponent::OnRep_ItemChanged()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APDPlayerController* PC = Cast<APDPlayerController>(OwnerPawn->GetController());
	if (PC)
	{
		PC->InitItemDataDisplay();
	}
}

// Called when the game starts
void UPDInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

