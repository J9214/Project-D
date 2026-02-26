#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassDelayRemovalProcessor.generated.h"


UCLASS()
class PROJECTD_API UMassDelayRemovalProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassDelayRemovalProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
