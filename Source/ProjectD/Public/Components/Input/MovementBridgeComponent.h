#pragma once

#include "CoreMinimal.h"
#include "Components/PawnExtensionComponentBase.h"
#include "MovementBridgeComponent.generated.h"

UENUM(BlueprintType)
enum class EMoveRequestType : uint8
{
	MoveTo,
	LinearVelocityDash,
	MoveLaunch,
};

USTRUCT(BlueprintType)
struct FMoveRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EMoveRequestType Type = EMoveRequestType::MoveTo;
	
	UPROPERTY(BlueprintReadWrite)
	FVector Start = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector Target = FVector::ZeroVector;  

	UPROPERTY(BlueprintReadWrite)
	FVector LaunchVelocity = FVector::ZeroVector;  
	
	UPROPERTY(BlueprintReadWrite)
	FName ForceMovementMode = EName::None;

	UPROPERTY(BlueprintReadWrite)
	float DurationMs = 0.f;  
	
	UPROPERTY(BlueprintReadWrite)
	uint8 Priority = 10;

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
