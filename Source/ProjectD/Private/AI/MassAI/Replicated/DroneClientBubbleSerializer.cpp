#include "AI/MassAI/Replicated/DroneClientBubbleSerializer.h"
#include "Net/Serialization/FastArraySerializer.h"

FDroneClientBubbleSerializer::FDroneClientBubbleSerializer()
	: FMassClientBubbleSerializerBase()
	, Agents()
	, Bubble()
{
}

void FDroneClientBubbleSerializer::Bind()
{
	Bubble.Initialize(Agents, *this);
	SetClientHandler(Bubble);
}

bool FDroneClientBubbleSerializer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FDroneFastArrayItem, FDroneClientBubbleSerializer>(Agents, DeltaParams, *this);
}