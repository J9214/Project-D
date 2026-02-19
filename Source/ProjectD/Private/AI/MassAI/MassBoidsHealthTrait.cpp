// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassBoidsHealthTrait.h"
#include "AI/MassAI/MassBoidsHealthFragment.h"
#include "MassEntityTemplateRegistry.h"

void UMassBoidsHealthTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FMassBoidsHealthFragment>();
}