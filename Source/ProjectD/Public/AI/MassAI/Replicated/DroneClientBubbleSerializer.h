#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleSerializerBase.h"
#include "AI/MassAI/Replicated/DroneClientBubbleHandler.h"
#include "AI/MassAI/Replicated/DroneFastArrayItem.h"
#include "DroneClientBubbleSerializer.generated.h"

USTRUCT()
struct FDroneClientBubbleSerializer : public FMassClientBubbleSerializerBase
{
	GENERATED_BODY()

public:
	FDroneClientBubbleSerializer();

public:
	void Bind();
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);

public:
	FORCEINLINE FDroneClientBubbleHandler& GetBubbleHandlerMutable() { return Bubble; }
	FORCEINLINE const TArray<FDroneFastArrayItem>& GetAgents() const { return Agents; }
	FORCEINLINE TArray<FDroneFastArrayItem>& GetAgentsMutable() { return Agents; }

private:
	UPROPERTY(Transient)
	TArray<FDroneFastArrayItem> Agents;
	FDroneClientBubbleHandler Bubble;
};

// For Delta Serialze
template<>
struct TStructOpsTypeTraits<FDroneClientBubbleSerializer> : public TStructOpsTypeTraitsBase2<FDroneClientBubbleSerializer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithCopy = false
	};
};