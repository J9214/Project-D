// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PDItemInfoSubsystem.generated.h"

struct FPDItemInfo;
/**
 * 
 */
UCLASS()
class PROJECTD_API UPDItemInfoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPDItemInfoSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	const FPDItemInfo* GetItemInfoByName(const FName& AttributeName) const;

	FORCEINLINE UDataTable* GetDataTable() const { return PDItemInfoDataTable; }
private:
	UPROPERTY()
	TObjectPtr<UDataTable> PDItemInfoDataTable;
};
