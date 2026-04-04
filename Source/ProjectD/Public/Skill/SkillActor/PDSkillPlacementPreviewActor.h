// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PDSkillPlacementPreviewActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;

UCLASS()
class PROJECTD_API APDSkillPlacementPreviewActor : public AActor
{
	GENERATED_BODY()
	
public:
	APDSkillPlacementPreviewActor();

	void InitializePreview(UStaticMesh* InMesh, UMaterialInterface* InMaterial, const FVector& InScale);
	void SetPlacementValid(bool bInValid);
	void SetPreviewTransform(const FTransform& InTransform);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PreviewMeshComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMID = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Preview")
	FName ValidParamName = TEXT("IsValid");
};