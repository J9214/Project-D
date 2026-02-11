#include "AI/MassAI/Replicated/DroneClientBubbleInfo.h"
#include "Net/UnrealNetwork.h"

ADroneClientBubbleInfo::ADroneClientBubbleInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DroneSerializer()
{
}

void ADroneClientBubbleInfo::PostInitProperties()
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		Serializers.Reset();
		Serializers.Add(&DroneSerializer);

		DroneSerializer.Bind();
	}

	Super::PostInitProperties();
}

void ADroneClientBubbleInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADroneClientBubbleInfo, DroneSerializer);
}