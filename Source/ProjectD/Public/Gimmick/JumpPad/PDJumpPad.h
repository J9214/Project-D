#pragma once

#include "CoreMinimal.h"
#include "Gimmick/PDOverlapGimmickBase.h"
#include "Mover/Public/DefaultMovementSet/LayeredMoves/BasicLayeredMoves.h"
#include "PDJumpPad.generated.h"

class USphereComponent;
class UPDJumpPadDataAsset;

UCLASS()
class PROJECTD_API APDJumpPad : public APDOverlapGimmickBase
{
	GENERATED_BODY()

public:
	APDJumpPad();

public:
	virtual void BeginPlay() override;

public:
	// IPDInteractableObject Interface
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void OnEndInteract_Implementation(AActor* Interactor) override;

protected:
	void LoadPDA();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gimmick|Component")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gimmick|JumpPad")
	FName PDAType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gimmick|JumpPad")
	FName PDAName;

protected:
	TSharedPtr<FLayeredMove_JumpTo> SPJump;
	UPDJumpPadDataAsset* JumpData;

private:
	static TSet<FName> PDANameSet;
};
