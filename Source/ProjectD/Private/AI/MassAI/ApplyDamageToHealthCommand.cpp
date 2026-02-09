#include "AI/MassAI/ApplyDamageToHealthCommand.h"

#include "MassEntityManager.h"   // FMassEntityManager (명시적으로 포함하는 게 안전)
#include "MassEntityView.h"      // FMassEntityView
#include "AI/MassAI/MassBoidsHealthFragment.h"

FApplyDamageToHealthCommand::FApplyDamageToHealthCommand()
	: FMassBatchedCommand(EMassCommandOperationType::Set, FName(TEXT("ApplyDamageToHealth")))
{
}

void FApplyDamageToHealthCommand::Add(const FMassEntityHandle InEntity, const float InDamage)
{
	if (InEntity.IsValid() == false ||
		InDamage <= 0.0f)
	{
		return;
	}

	Entities.Add(InEntity);
	Damages.Add(InDamage);
}

void FApplyDamageToHealthCommand::Run(FMassEntityManager& EntityManager)
{
	const int32 Count = Entities.Num();
	for (int32 i = 0; i < Count; ++i)
	{
		const FMassEntityHandle Entity = Entities[i];
		const float Damage = Damages[i];

		if (EntityManager.IsEntityValid(Entity) == false)
		{
			continue;
		}

		FMassEntityView View = FMassEntityView::TryMakeView(EntityManager, Entity);
		if (View.IsValid() == false)
		{
			continue;
		}

		FMassBoidsHealthFragment* HealthFrag = View.GetFragmentDataPtr<FMassBoidsHealthFragment>();
		if (HealthFrag == nullptr)
		{
			continue;
		}

		HealthFrag->Health = FMath::Clamp(HealthFrag->Health - Damage, 0.0f, HealthFrag->MaxHealth);
	}
}

void FApplyDamageToHealthCommand::Reset()
{
	Entities.Reset();
	Damages.Reset();
}
