// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/PDServerLobbyPlayerController.h"
#include "Blueprint/UserWidget.h"

void APDServerLobbyPlayerController::BeginPlay()
{

    if (IsLocalController() == false)
    {
        return;
    }

    if (UIWidgetClass)
    {
        UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass);
        if (UIWidgetInstance)
        {
            UIWidgetInstance->AddToViewport();
            UIWidgetInstance->SetVisibility(ESlateVisibility::Visible);
        }
    }

    bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    SetInputMode(InputMode);
}
