// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance/Subsystem/PDItemInfoSubsystem.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/Shop/FPDItemInfo.h"


UPDItemInfoSubsystem::UPDItemInfoSubsystem()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> PDItemInfo(TEXT("/Game/ProjectD/Data/Item/DT_PDItemInfo.DT_PDItemInfo"));
	if (PDItemInfo.Succeeded())
	{
		PDItemInfoDataTable = PDItemInfo.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UPDPDItemInfoSubsystem::UPDItemInfoSubsystem - Failed to find PDItemInfoDataTable at specified path."));
	}
}

void UPDItemInfoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!IsValid(PDItemInfoDataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("UPDItemInfoSubsystem::Initialize - PDItemInfoDataTable is null. Check path."));
	}
}

void UPDItemInfoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

const FPDItemInfo* UPDItemInfoSubsystem::GetItemInfoByName(const FName& AttributeName) const
{
	if (AttributeName.IsNone())
	{
		return nullptr;
	}

	if (!IsValid(PDItemInfoDataTable))
	{
		return nullptr;
	}

	return PDItemInfoDataTable->FindRow<FPDItemInfo>(AttributeName, TEXT("UPDItemInfoSubsystem::GetPDItemInfo"));
}
