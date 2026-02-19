// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassBoidsHealthTrait.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Boids Health"))
class PROJECTD_API UMassBoidsHealthTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
