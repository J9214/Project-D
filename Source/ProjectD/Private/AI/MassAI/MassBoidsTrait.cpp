// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassBoidsTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "AI/MassAI/MassTargetFragment.h"
#include "StructUtils/SharedStruct.h"
#include <MassActorSubsystem.h>

void UMassBoidsTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment<FMassVelocityFragment>();
	BuildContext.AddFragment<FMassTargetFragment>();

	BuildContext.AddSharedFragment(FSharedStruct::Make(BoidsSettings));
}
