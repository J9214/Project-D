// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PDListenLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTD_API APDListenLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
};
