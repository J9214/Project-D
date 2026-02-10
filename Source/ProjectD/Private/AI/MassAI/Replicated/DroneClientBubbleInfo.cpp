#include "AI/MassAI/Replicated/DroneClientBubbleInfo.h"
#include "Net/UnrealNetwork.h"

ADroneClientBubbleInfo::ADroneClientBubbleInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DroneSerializer()
{
	Serializers.Add(&DroneSerializer);
}

void ADroneClientBubbleInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADroneClientBubbleInfo, DroneSerializer);
}