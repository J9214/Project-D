#include "AI/MassAI/CheckDroneAllExplodeSubsystem.h"

void UCheckDroneAllExplodeSubsystem::RequestExplodeAllDrones()
{
	bExplodeAllRequested = true;
}

bool UCheckDroneAllExplodeSubsystem::ConsumeExplodeAllRequest()
{
	if (bExplodeAllRequested == false)
	{
		return false;
	}

	bExplodeAllRequested = false;
	return true;
}
