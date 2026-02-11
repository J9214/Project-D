#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DroneReplicationBootingSubsystem.generated.h"

UCLASS()
class PROJECTD_API UDroneReplicationBootingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool ShouldRunForThisWorld() const;

private:
	UPROPERTY()
	bool bRegisteredBubbleClass = false;
};
