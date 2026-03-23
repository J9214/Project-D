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

void UPDIngameInfo::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	APDPawnBase* Pawn = Cast<APDPawnBase>(PC->GetPawn());
	if (!Pawn)
	{
		return;
	}

	UWeaponManageComponent* WeaponComp = Pawn->GetWeaponManageComponent();
	if (!WeaponComp)
	{
		return;
	}

	WeaponComp->OnEquippedWeaponChanged.AddUObject(this, &ThisClass::HandleEquippedWeaponChanged);
	WeaponComp->OnEquippedThrowableChanged.AddUObject(this, &ThisClass::HandleEquippedThrowableChanged);

}

void UPDIngameInfo::SetActiveWeapon(const FSlateBrush& InBrush, int32 InCurrentBullet, int32 InMaxBullet, int32 InSlotIndex)
{
	if (ActiveWeapon)
	{
		ActiveWeapon->SetBrush(InBrush);
	}

	if (CurrentBullet)
	{
		CurrentBullet->SetText(FText::AsNumber(InCurrentBullet));
	}

	if (MaxBullet)
	{
		MaxBullet->SetText(FText::AsNumber(InMaxBullet));
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

void UPDIngameInfo::HandleEquippedWeaponChanged(APDWeaponBase* NewWeapon)
{

	//if (!NewWeapon || !NewWeapon->WeaponData)
	//{
	//	if (ActiveWeapon)
	//	{
	//		ActiveWeapon->SetBrush(FSlateBrush());
	//	}

	//	if (CurrentBullet)
	//	{
	//		CurrentBullet->SetText(FText::GetEmpty());
	//	}

	//	if (MaxBullet)
	//	{
	//		MaxBullet->SetText(FText::GetEmpty());
	//	}

	//	return;
	//}

	//const FName WeaponItemID = NewWeapon->WeaponData->ItemID;

	//UTexture2D* IconTex = GetItemIconTextureByID(WeaponItemID);
	//if (ActiveWeapon)
	//{
	//	if (IconTex)
	//	{
	//		ActiveWeapon->SetBrushFromTexture(IconTex, true);
	//	}
	//	else
	//	{
	//		ActiveWeapon->SetBrush(FSlateBrush());
	//	}
	//}

	//if (CurrentBullet)
	//{
	//	CurrentBullet->SetText(FText::AsNumber(NewWeapon->GetCurrentAmmo()));
	//}

	//if (MaxBullet)
	//{
	//	MaxBullet->SetText(FText::AsNumber(NewWeapon->GetMaxAmmo()));
	//}

}

void UPDIngameInfo::HandleEquippedThrowableChanged(APDThrowableItemBase* NewThrowable)
{/*
	if (!NewThrowable)
	{
		if (ActiveGrenade)
		{
			ActiveGrenade->SetBrush(FSlateBrush());
		}
		return;
	}

	const FName ThrowableItemID = NewThrowable->ItemID;

	UTexture2D* IconTex = GetItemIconTextureByID(ThrowableItemID);
	if (ActiveGrenade)
	{
		if (IconTex)
		{
			ActiveGrenade->SetBrushFromTexture(IconTex, true);
		}
		else
		{
			ActiveGrenade->SetBrush(FSlateBrush());
		}
	}*/
}

UTexture2D* UPDIngameInfo::GetItemIconTextureByID(const FName& ItemID) const
{
	//if (ItemID.IsNone())
	//{
	//	return nullptr;
	//}

	//UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	//if (!GI)
	//{
	//	return nullptr;
	//}

	//UPDItemInfoSubsystem* ItemSubsystem = GI->GetSubsystem<UPDItemInfoSubsystem>();
	//if (!ItemSubsystem)
	//{
	//	return nullptr;
	//}

	//const FPDItemInfo* Info = ItemSubsystem->GetItemInfoByName(ItemID);
	//if (!Info)
	//{
	//	return nullptr;
	//}

	//return Info->IconImage.LoadSynchronous();

	return nullptr;
}
