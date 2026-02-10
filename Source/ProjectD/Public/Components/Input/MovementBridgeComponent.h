#pragma once

#include "CoreMinimal.h"
#include "Components/PawnExtensionComponentBase.h"
#include "MovementBridgeComponent.generated.h"

UENUM(BlueprintType)
enum class EMoveRequestType : uint8
{
	MoveToDash,
	LinearVelocityDash
};

USTRUCT(BlueprintType)
struct FMoveRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EMoveRequestType Type = EMoveRequestType::MoveToDash;
	
	UPROPERTY(BlueprintReadWrite)
	FVector Start = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	FVector Target = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float DurationSec = 0.5f;
	
	UPROPERTY(BlueprintReadWrite)
	int32 Priority = 10;

	UPROPERTY(BlueprintReadWrite)
	bool bCancelExisting = false; // if true cancel existing move request
};

UCLASS()
class PROJECTD_API UMovementBridgeComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void EnqueueMoveRequest(const FMoveRequest& MoveRequest);
	
	UFUNCTION(BlueprintCallable)
	void ConsumeMoveRequest(TArray<FMoveRequest>& OutRequests);
	
	UFUNCTION(BlueprintCallable)
	void ClearMoveRequests();
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FMoveRequest> PendingMoveRequests;
};
