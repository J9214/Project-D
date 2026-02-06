// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlayerLocSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTD_API UPlayerLocSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void GetPlayerControllers(TArray<class APlayerController*>& OutControllers) const;
	void GetPlayerPawns(TArray<class APawn*>& OutPawns) const;
	void GetPlayerLocations(TArray<FVector>& OutLocations) const;
	void GetPlayerViewpoints(TArray<FVector>& OutOrigins, TArray<FVector>& OutDirs) const;
};
