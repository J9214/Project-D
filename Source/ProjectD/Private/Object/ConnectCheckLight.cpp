// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/ConnectCheckLight.h"
#include "GameState/PDLobbyGameState.h"
#include "Engine/World.h"
#include "Components/PointLightComponent.h"

AConnectCheckLight::AConnectCheckLight()
{
	bReplicates = false;
    PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLightComponent"));
    RootComponent = PointLightComponent;
}

void AConnectCheckLight::BeginPlay()
{
    Super::BeginPlay();

    if (GetNetMode() != NM_DedicatedServer)
    {
        RegisterToGameState();
    }

    UE_LOG(LogTemp, Warning, TEXT("Calling BP_OnSlotStateChanged..."));
    BP_OnSlotStateChanged(true);
}

void AConnectCheckLight::SetLightColorLocal(FLinearColor NewColor)
{
    if (PointLightComponent)
    {
        PointLightComponent->SetLightColor(NewColor);
        PointLightComponent->SetVisibility(true);
    }
}

void AConnectCheckLight::RegisterToGameState()
{
    if (APDLobbyGameState* GS = GetWorld()->GetGameState<APDLobbyGameState>())
    {
        GS->RegisterLobbyLight(this);
    }
    else
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &AConnectCheckLight::RegisterToGameState);
    }
}
