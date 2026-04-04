// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Ingame/PDIngameInfo.h"
#include "GameInstance/Subsystem/PDItemInfoSubsystem.h"
#include "Components/Shop/FPDItemInfo.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Weapon/PDWeaponBase.h"
#include "Weapon/PDThrowableItemBase.h"
#include <Pawn/PDPawnBase.h>
#include "Components/Combat/WeaponManageComponent.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"

void UPDIngameInfo::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::NativeOnInitialized PC Null"));
		return;
	}

	APDPawnBase* Pawn = Cast<APDPawnBase>(PC->GetPawn());
	if (!Pawn)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::NativeOnInitialized Pawn Null"));
		return;
	}

	UWeaponManageComponent* WeaponComp = Pawn->GetWeaponManageComponent();
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::NativeOnInitialized WeaponComp Null"));
		return;
	}

	WeaponComp->OnEquippedWeaponChanged.AddUObject(this, &ThisClass::HandleEquippedWeaponChanged);
	WeaponComp->OnEquippedThrowableChanged.AddUObject(this, &ThisClass::HandleEquippedThrowableChanged);

}

void UPDIngameInfo::SetActiveWeapon(const FSlateBrush& InBrush, int32 InCurrentAmmo, int32 InMaxAmmo, int32 InSlotIndex)
{
	if (ActiveWeapon)
	{
		ActiveWeapon->SetBrush(InBrush);
	}

	if (CurrentAmmo)
	{
		CurrentAmmo->SetText(FText::AsNumber(InCurrentAmmo));
	}

	if (MaxAmmo)
	{
		MaxAmmo->SetText(FText::AsNumber(InMaxAmmo));
	}

	if (ActiveSlotIndex)
	{
		ActiveSlotIndex->SetText(FText::AsNumber(InSlotIndex));
	}
}

void UPDIngameInfo::SetActiveGrenade(const FSlateBrush& InBrush)
{
	if (ActiveGrenada)
	{
		ActiveGrenada->SetBrush(InBrush);
	}
}

void UPDIngameInfo::SetSkillIcon(int32 SkillIndex, const FSlateBrush& InBrush)
{
	if (SkillIndex == 0 && Skill1)
	{
		Skill1->SetBrush(InBrush);
	}
	else if (SkillIndex == 1 && Skill2)
	{
		Skill2->SetBrush(InBrush);
	}
}

void UPDIngameInfo::UpdateCurrentAmmo(int32 AmmoCount)
{
	if (CurrentAmmo)
	{
		CurrentAmmo->SetText(FText::AsNumber(AmmoCount));
	}
}

void UPDIngameInfo::HandleEquippedWeaponChanged(APDWeaponBase* NewWeapon)
{

	if (!NewWeapon || !NewWeapon->WeaponData)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::HandleEquippedWeaponChanged NewWeapon Null"));
		return;
	}

	const FName WeaponItemID = NewWeapon->WeaponData->ItemID;

	UTexture2D* IconTex = GetItemIconTextureByID(WeaponItemID);
	if (ActiveWeapon)
	{
		if (IconTex)
		{
			ActiveWeapon->SetBrushFromTexture(IconTex, true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::HandleEquippedWeaponChanged IconTex Null"));
			ActiveWeapon->SetBrushFromTexture(EmptyWeaponIconTexture, true);
		}
	}

	if (CurrentAmmo)
	{
		CurrentAmmo->SetText(FText::AsNumber(NewWeapon->GetCurrentAmmo()));
	}

	if (MaxAmmo)
	{
		MaxAmmo->SetText(FText::AsNumber(NewWeapon->GetMaxAmmo()));
	}

}

void UPDIngameInfo::HandleEquippedThrowableChanged(APDThrowableItemBase* NewThrowable)
{
	if (!NewThrowable || !NewThrowable->GetThrowableData())
	{
		if (ActiveGrenada)
		{
			UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::HandleEquippedThrowableChanged NewThrowable Null"));
		}
		return;
	}

	const FName ThrowableItemID = NewThrowable->GetThrowableData()->ItemID;

	UTexture2D* IconTex = GetItemIconTextureByID(ThrowableItemID);
	if (ActiveGrenada)
	{
		if (IconTex)
		{
			ActiveGrenada->SetBrushFromTexture(IconTex, true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::HandleEquippedThrowableChanged IconTex Null"));
			ActiveGrenada->SetBrushFromTexture(EmptyThrowIconTexture, true);
		}
	}
}

UTexture2D* UPDIngameInfo::GetItemIconTextureByID(const FName& ItemID) const
{
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::GetItemIconTextureByID ItemID Null"));
		return nullptr;
	}

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::GetItemIconTextureByID GI Null"));
		return nullptr;
	}

	UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
	if (!ItemSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::GetItemIconTextureByID ItemSubsystem Null"));
		return nullptr;
	}

	const FPDItemInfo* Info = ItemSubsystem->GetItemInfoByName(ItemID);
	if (!Info)
	{
		UE_LOG(LogTemp, Error, TEXT("UPDIngameInfo::GetItemIconTextureByID Info Null"));
		return nullptr;
	}

	return Info->IconImage.LoadSynchronous();
}
