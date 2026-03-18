#pragma once

#include "CoreMinimal.h"
#include "Gimmick/PDDirectlyInteractGimmickBase.h"
#include "Mover/Public/DefaultMovementSet/LayeredMoves/BasicLayeredMoves.h"
#include "GameplayTagContainer.h"
#include "PDZipLine.generated.h"

class UBoxComponent;
class UCableComponent;
class APDPawnBase;
class UMovementBridgeComponent;

USTRUCT(BlueprintType)
struct FLayeredMove_ZipLineVelocity : public FLayeredMove_LinearVelocity
{
	GENERATED_BODY()

	virtual void OnEnd(const UMoverComponent* MoverComp, UMoverBlackboard* SimBlackboard, double CurrentSimTimeMs) override
	{
		DurationMs = CurrentSimTimeMs - 1.0f;
	}
};

USTRUCT(BlueprintType)
struct FZipLineInfo
{
	GENERATED_BODY()

	UMovementBridgeComponent* BridgeComp;
	FTimerHandle Timer;
	FVector LastVelocity;
	bool bIsZiplining = false;
};

UCLASS()
class PROJECTD_API APDZipLine : public APDDirectlyInteractGimmickBase
{
	GENERATED_BODY()

public:
	APDZipLine();

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndReason) override;

public:
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual void OnEndInteract_Implementation(AActor* Interactor) override;

protected:
	FVector CalcRealDirection(APawn* User);
	float MoveUser(APawn* User);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Component")
	TObjectPtr<UStaticMeshComponent> OtherStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Component")
	TObjectPtr<UCableComponent> Cable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Component")
	TObjectPtr<UBoxComponent> InteractBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Component")
	TObjectPtr<USceneComponent> CableAttachA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Component")
	TObjectPtr<USceneComponent> CableAttachB;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gimmick|ZipLine")
	FGameplayTag ZipLineTag;

	FVector Direction;
	float Speed = 1500.0f;
	float Duration = -1.0f;
	TMap<APDPawnBase*, FZipLineInfo> ZipLineLayeredMoveMap;
};
