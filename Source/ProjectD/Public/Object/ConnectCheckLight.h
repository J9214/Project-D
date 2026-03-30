// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ConnectCheckLight.generated.h"

class UPointLightComponent;

UENUM(BlueprintType)
enum class EPDLobbyTeam : uint8
{
	None = 0,
	TeamA = 1,
	TeamB = 2
};
/**
 * 
 */
UCLASS()
class PROJECTD_API AConnectCheckLight : public AActor
{
	GENERATED_BODY()
	
public:
	AConnectCheckLight();
protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> PointLightComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LobbySettings")
	EPDLobbyTeam AssignedTeam = EPDLobbyTeam::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LobbySettings")
	int32 LightIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "LobbySettings")
	void SetLightColorLocal(FLinearColor NewColor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void BP_OnSlotStateChanged(bool bIsActive);

private:
	void RegisterToGameState();
};
