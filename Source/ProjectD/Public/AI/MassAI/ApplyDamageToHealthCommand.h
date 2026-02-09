#pragma once

#include "CoreMinimal.h"
#include "MassCommands.h" 
#include "MassEntityTypes.h"

struct PROJECTD_API FApplyDamageToHealthCommand : public FMassBatchedCommand
{
public:
	FApplyDamageToHealthCommand();

	void Add(const FMassEntityHandle InEntity, const float InDamage);

	virtual void Run(FMassEntityManager& EntityManager) override;
	virtual void Reset() override;

protected:
	virtual SIZE_T GetAllocatedSize() const override { return sizeof(*this); }
	virtual int32 GetNumOperationsStat() const override { return Entities.Num(); }

private:
	TArray<FMassEntityHandle> Entities;
	TArray<float> Damages;
};
