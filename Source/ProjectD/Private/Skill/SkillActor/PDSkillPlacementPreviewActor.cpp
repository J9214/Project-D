

#include "Skill/SkillActor/PDSkillPlacementPreviewActor.h"

APDSkillPlacementPreviewActor::APDSkillPlacementPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMeshComponent"));
	PreviewMeshComponent->SetupAttachment(Root);
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetGenerateOverlapEvents(false);
	PreviewMeshComponent->SetCanEverAffectNavigation(false);
}

void APDSkillPlacementPreviewActor::InitializePreview(UStaticMesh* InMesh, UMaterialInterface* InMaterial, const FVector& InScale)
{
	if (InMesh)
	{
		PreviewMeshComponent->SetStaticMesh(InMesh);
	}

	SetActorScale3D(InScale);

	if (InMaterial)
	{
		PreviewMID = UMaterialInstanceDynamic::Create(InMaterial, this);
		if (PreviewMID)
		{
			PreviewMeshComponent->SetMaterial(0, PreviewMID);
		}
		else
		{
			PreviewMeshComponent->SetMaterial(0, InMaterial);
		}
	}
}

void APDSkillPlacementPreviewActor::SetPlacementValid(bool bInValid)
{
	if (PreviewMID)
	{
		PreviewMID->SetScalarParameterValue(ValidParamName, bInValid ? 1.f : 0.f);
	}
}

void APDSkillPlacementPreviewActor::SetPreviewTransform(const FTransform& InTransform)
{
	SetActorTransform(InTransform);
}

