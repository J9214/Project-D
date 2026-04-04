#pragma once

#include "CoreMinimal.h"
#include "Gimmick/PDOverlapGimmickBase.h"
#include "PDTeleporter.generated.h"

class UBoxComponent;

UCLASS()
class PROJECTD_API APDTeleporter : public APDOverlapGimmickBase
{
	GENERATED_BODY()

public:
	APDTeleporter();
	
public:
	// IPDInteractableObject Interface
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void OnEndInteract_Implementation(AActor* Interactor) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gimmick|Component")
	TObjectPtr<UBoxComponent> BoxComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Teleport")
	TObjectPtr<USceneComponent> Destination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Teleport")
	TWeakObjectPtr<APDTeleporter> TeleportDest;
};
