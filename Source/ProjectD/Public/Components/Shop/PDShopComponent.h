// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Shop/FPDItemInfo.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "PDShopComponent.generated.h"

UENUM(BlueprintType)
enum class EShopBuyResult : uint8
{
	Success,
	NotEnoughGold,
	OutOfStock,
	InvalidItem
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTD_API UPDShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPDShopComponent();

	UFUNCTION(BlueprintCallable)
	void RequestBuy(FName ItemId);
protected:

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_AcceptanceBuy(FName ItemId);

};
