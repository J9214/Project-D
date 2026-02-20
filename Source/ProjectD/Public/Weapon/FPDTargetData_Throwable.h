#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "FPDTargetData_Throwable.generated.h"

USTRUCT()
struct PROJECTD_API FPDTargetData_Throwable : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector_NetQuantize10 StartLocation;

	UPROPERTY()
	FVector_NetQuantize10 InitialVelocity;

	UPROPERTY()
	float FuseTime = 3.0f;

	UPROPERTY()
	float GravityScale = 1.0f;

	virtual UScriptStruct* GetScriptStruct() const override { return FPDTargetData_Throwable::StaticStruct(); }
	virtual bool HasOrigin() const override { return true; }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << StartLocation;
		Ar << InitialVelocity;
		Ar << FuseTime;
		Ar << GravityScale;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FPDTargetData_Throwable> : public TStructOpsTypeTraitsBase2<FPDTargetData_Throwable>
{
	enum { WithNetSerializer = true };
};