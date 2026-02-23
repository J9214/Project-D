// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TestPlayerController.generated.h"

class UUserWidget;
/**
 * 
 */
UCLASS()
class PROJECTD_API ATestPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess))
    TSubclassOf<UUserWidget> UIWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess))
    TObjectPtr<UUserWidget> UIWidgetInstance;
};
