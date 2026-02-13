// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PDShopComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTD_API UPDShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPDShopComponent();

	//UFUNCTION(BlueprintCallable, Category = "Shop|Data")
	//UDataTable* GetItemDataTable() const;

protected:

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Data")
	//TSoftObjectPtr<UDataTable> ItemDataTable;


};
