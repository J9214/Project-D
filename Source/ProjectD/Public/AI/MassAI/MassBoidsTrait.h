// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassBoidsFragment.h"
#include "MassBoidsTrait.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTD_API UMassBoidsTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Boids")
	FMassBoidsFragment BoidsSettings;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
