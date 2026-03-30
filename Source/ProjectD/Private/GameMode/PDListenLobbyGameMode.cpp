// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/PDListenLobbyGameMode.h"
#include "Controller/PDLobbyPlayerController.h"
#include <PlayerState/PDPlayerState.h>

void APDListenLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void APDListenLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (auto* PC = Cast<APDLobbyPlayerController>(NewPlayer))
    {
        PC->Client_RequestCharacterCustomInfo();
        PC->Client_RequestDisplayName();
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APDLobbyPlayerController* AllPC = Cast<APDLobbyPlayerController>(It->Get()))
        {
            AllPC->Client_UpdateLobbyUI();
        }
    }
}
