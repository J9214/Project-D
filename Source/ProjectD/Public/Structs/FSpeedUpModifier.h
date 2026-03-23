#pragma once

#include "CoreMinimal.h"
#include "MovementModifier.h"
#include "FSpeedUpModifier.generated.h"

USTRUCT(BlueprintType)
struct PROJECTD_API FSpeedUpModifier : public FMovementModifierBase
{
	GENERATED_BODY()

public:
	virtual FMovementModifierBase* Clone() const override
	{
		return new FSpeedUpModifier(*this);
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual void NetSerialize(FArchive& Ar) override
	{
		Super::NetSerialize(Ar);
		Ar << SpeedMultiplier;
	}

	virtual FString ToSimpleString() const override
	{
		return FString::Printf(TEXT("FSpeedUpModifier Multiplier=%.2f DurationMs=%.2f"), SpeedMultiplier, DurationMs);
	}
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpeedUp")
	float SpeedMultiplier = 1.2f;
};