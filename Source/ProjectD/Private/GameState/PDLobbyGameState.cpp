// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/PDLobbyGameState.h"
#include <Object/ConnectCheckLight.h>

void APDLobbyGameState::RegisterLobbyLight(AConnectCheckLight* InLight)
{
	if (!InLight)
	{
		return;
	}

	if (InLight->AssignedTeam == EPDLobbyTeam::None)
	{
		PlayerTeamLight = InLight;
	}
	else if (InLight->AssignedTeam == EPDLobbyTeam::TeamA)
	{
		if (InLight->LightIndex == 0)
		{
			TeamALight1 = InLight;
		}
		else if (InLight->LightIndex == 1)
		{
			TeamALight2 = InLight;
		}
	}
	else if (InLight->AssignedTeam == EPDLobbyTeam::TeamB)
	{
		if (InLight->LightIndex == 0)
		{
			TeamBLight1 = InLight;
		}
		else if (InLight->LightIndex == 1)
		{
			TeamBLight2 = InLight;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GameState: Registered Light for Team %d, Index %d"),
		(uint8)InLight->AssignedTeam, InLight->LightIndex);
}
