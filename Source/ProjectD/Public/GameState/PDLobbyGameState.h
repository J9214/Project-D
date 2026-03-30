// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PDLobbyGameState.generated.h"

class AConnectCheckLight;
/**
 * 
 */
UCLASS()
class PROJECTD_API APDLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
public:

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Lobby|Lights")
	TObjectPtr<AConnectCheckLight> PlayerTeamLight;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Lobby|Lights")
	TObjectPtr<AConnectCheckLight> TeamALight1;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Lobby|Lights")
	TObjectPtr<AConnectCheckLight> TeamALight2;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Lobby|Lights")
	TObjectPtr<AConnectCheckLight> TeamBLight1;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Lobby|Lights")
	TObjectPtr<AConnectCheckLight> TeamBLight2;

	void RegisterLobbyLight(AConnectCheckLight* InLight);
};
