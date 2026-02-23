#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PDInteractableObject.h"
#include "PDGimmickBase.generated.h"

UCLASS(Abstract)
class PROJECTD_API APDGimmickBase : public AActor, public IPDInteractableObject
{
	GENERATED_BODY()
	
public:
	APDGimmickBase();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnInteract_Implementation(AActor* Interactor) PURE_VIRTUAL(APDGimmickBase::OnInteract_Implementation, );
	virtual void OnEndInteract_Implementation(AActor* Interactor) PURE_VIRTUAL(APDGimmickBase::OnEndInteract_Implementation, );

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gimmick|Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gimmick|Component")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Interact")
	bool bIsCanInteract;
};
