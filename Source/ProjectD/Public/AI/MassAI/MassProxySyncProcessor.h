#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassProxySyncProcessor.generated.h"

class UMassProxyPoolSubsystem;

UCLASS()
class PROJECTD_API UMassProxySyncProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	UMassProxySyncProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	UMassProxyPoolSubsystem* ProxyPoolSubsystem = nullptr;

	UPROPERTY(EditAnywhere, Category = "Proxy|Sync")
	float UpdateIntervalSeconds = 0.0f;

	float TimeSinceLastUpdateSeconds = 0.0f;
};
