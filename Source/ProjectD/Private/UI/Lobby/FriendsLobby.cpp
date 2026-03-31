// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/FriendsLobby.h"
#include "Components/Button.h"

void UFriendsLobby::NativeConstruct()
{
    Super::NativeConstruct();

    if (GameStart)
    {
        APlayerController* PC = GetOwningPlayer();

        if (PC && !PC->HasAuthority())
        {
            GameStart->SetVisibility(ESlateVisibility::Collapsed);
        }
        else
        {
            GameStart->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void UFriendsLobby::InitFriendsPopupButton(bool Active)
{
	FriendsPopupButton->SetVisibility(Active ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UFriendsLobby::ClosePopupAnimation()
{
    if (CloseFriendsList)
    {
        PlayAnimation(CloseFriendsList);
    }

    IsActiveFriendsPopup = false;
}

void UFriendsLobby::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (FriendsPopupButton)
    {
        FriendsPopupButton->OnClicked.AddDynamic(this, &ThisClass::HandleFriendsPopupClicked);
    }
}

void UFriendsLobby::HandleFriendsPopupClicked()
{
    if(IsActiveFriendsPopup)
    {
        if (CloseFriendsList)
        {
            PlayAnimation(CloseFriendsList);
        }
    }
    else
    {
        if (OpenFriendsList)
        {
            PlayAnimation(OpenFriendsList);
        }
    }

}
