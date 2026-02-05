// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassBoidsDestructionProcessor.h"
#include "AI/MassAI/MassBoidsHealthFragment.h" 
#include "MassCommonFragments.h"     
#include "MassExecutionContext.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UMassBoidsDestructionProcessor::UMassBoidsDestructionProcessor()
	:EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UMassBoidsDestructionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassBoidsHealthFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsDestructionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			TConstArrayView<FMassBoidsHealthFragment> Healths = Context.GetFragmentView<FMassBoidsHealthFragment>();
			TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();

			const int32 NumEntities = Context.GetNumEntities();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				if (Healths[i].Health <= 0.0f)
				{
					FMassEntityHandle Entity = Context.GetEntity(i);
					FVector DeathLocation = Transforms[i].GetTransform().GetLocation();

					if (ExplosionEffect)
					{
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(
							GetWorld(),
							ExplosionEffect,
							DeathLocation,
							FRotator::ZeroRotator,
							EffectScale,
							true, // Auto Destroy
							true, // Auto Activate
							ENCPoolMethod::AutoRelease // Use Pooling
						);
					}

					if (ExplosionSound)
					{
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, DeathLocation);
					}

					Context.Defer().DestroyEntity(Entity);
				}
			}
		});
}
