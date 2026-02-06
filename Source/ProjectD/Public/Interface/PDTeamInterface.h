// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PDTeamInterface.generated.h"

UENUM(BlueprintType)
enum class ETeamType : uint8
{
	None UMETA(DisplayName = "None"),
	TeamOne  UMETA(DisplayName = "TeamOne"),
	TeamTwo UMETA(DisplayName = "TeamTwo"),
	TeamThree UMETA(DisplayName = "TeamThree")

};

UINTERFACE(MinimalAPI)
class UPDTeamInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTD_API IPDTeamInterface
{
	GENERATED_BODY()

public:
	virtual ETeamType GetTeamID() const = 0;
};
