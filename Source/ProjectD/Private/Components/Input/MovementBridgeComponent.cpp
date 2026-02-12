#include "Components/Input/MovementBridgeComponent.h"

void UMovementBridgeComponent::EnqueueMoveRequest(const FMoveRequest& MoveRequest)
{
	if (MoveRequest.bCancelExisting)
	{
		PendingMoveRequests.Reset();
	}
	
	PendingMoveRequests.Add(MoveRequest);
}

void UMovementBridgeComponent::ConsumeMoveRequest(TArray<FMoveRequest>& OutRequests)
{
	OutRequests.Reset();
	OutRequests = PendingMoveRequests;
	PendingMoveRequests.Reset();
	
	OutRequests.Sort([](const FMoveRequest& A, const FMoveRequest& B)
	{
		return A.Priority > B.Priority;
	});
}

void UMovementBridgeComponent::ClearMoveRequests()
{
	PendingMoveRequests.Reset();
}


