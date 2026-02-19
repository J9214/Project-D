#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleInfoBase.h"
#include "AI/MassAI/Replicated/DroneClientBubbleSerializer.h"
#include "DroneClientBubbleInfo.generated.h"

UCLASS()
class PROJECTD_API ADroneClientBubbleInfo : public AMassClientBubbleInfoBase
{
	GENERATED_BODY()

public:
	ADroneClientBubbleInfo(const FObjectInitializer& ObjectInitializer);

public:
	FORCEINLINE FDroneClientBubbleSerializer& GetSerializerMutable() { return DroneSerializer; }
	FORCEINLINE const FDroneClientBubbleSerializer& GetSerializer() const { return DroneSerializer; }

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UPROPERTY(Replicated, Transient)
	FDroneClientBubbleSerializer DroneSerializer;
};