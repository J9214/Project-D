// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PDServerLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTD_API APDServerLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    virtual void BeginPlay() override;

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI)
    TSubclassOf<UUserWidget> UIWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI)
    TObjectPtr<UUserWidget> UIWidgetInstance;
};
