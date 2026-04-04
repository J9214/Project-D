#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Mover/Public/DefaultMovementSet/LayeredMoves/BasicLayeredMoves.h"
#include "PDJumpPadDataAsset.generated.h"

UCLASS()
class PROJECTD_API UPDJumpPadDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("JumpPad", GetFName());
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|JumpPad")
	FLayeredMove_JumpTo JumpInfo;
};
