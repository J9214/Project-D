#include "AI/MassAI/Replicated/DroneFastArrayItem.h"

FDroneFastArrayItem::FDroneFastArrayItem()
	: FMassFastArrayItemBase()
	, Agent()
{
}

FDroneFastArrayItem::FDroneFastArrayItem(const FDroneReplicatedAgent& InAgent, const FMassReplicatedAgentHandle InHandle)
	: FMassFastArrayItemBase(InHandle)
	, Agent(InAgent)
{
}