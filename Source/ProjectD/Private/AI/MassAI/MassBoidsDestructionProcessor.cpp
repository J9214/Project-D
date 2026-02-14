// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassBoidsDestructionProcessor.h"
#include "AI/MassAI/MassBoidsHealthFragment.h" 
#include "AI/MassAI/MassProxyPoolSubsystem.h" 
#include "MassCommonFragments.h"     
#include "MassExecutionContext.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AI/MassAI/MassDamageBridgeSubsystem.h"
#include "MassEntityView.h"

UMassBoidsDestructionProcessor::UMassBoidsDestructionProcessor()
	:EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UMassBoidsDestructionProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	if (ExplosionEffectAsset.IsNull() == false)
	{
		ExplosionEffect = ExplosionEffectAsset.LoadSynchronous();
	}

	if (ExplosionSoundAsset.IsNull() == false)
	{
		ExplosionSound = ExplosionSoundAsset.LoadSynchronous();
	}
}

void UMassBoidsDestructionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassBoidsHealthFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsDestructionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == true)
	{
		UMassDamageBridgeSubsystem* Bridge = World->GetSubsystem<UMassDamageBridgeSubsystem>();
		if (IsValid(Bridge) == true)
		{
			TArray<FPendingMassDamage> Requests;
			Bridge->MovePendingDamages(Requests);

			for (const FPendingMassDamage& Req : Requests)
			{
				if (Req.Entity.IsValid() == false || Req.Damage <= 0.0f)
				{
					continue;
				}

				if (EntityManager.IsEntityValid(Req.Entity) == false)
				{
					continue;
				}

				FMassEntityView View = FMassEntityView::TryMakeView(EntityManager, Req.Entity);
				if (View.IsValid() == false)
				{
					continue;
				}

				FMassBoidsHealthFragment* HealthFrag = View.GetFragmentDataPtr<FMassBoidsHealthFragment>();
				if (HealthFrag == nullptr)
				{
					continue;
				}

				HealthFrag->Health = FMath::Clamp(HealthFrag->Health - Req.Damage, 0.0f, HealthFrag->MaxHealth);
			}
		}
	}

	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			const TConstArrayView<FMassBoidsHealthFragment> Healths = Context.GetFragmentView<FMassBoidsHealthFragment>();
			const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();

			const int32 NumEntities = Context.GetNumEntities();
			for (int32 i = 0; i < NumEntities; ++i)
			{
				if (Healths[i].Health <= 0.0f)
				{
					const FMassEntityHandle Entity = Context.GetEntity(i);
					const FVector DeathLocation = Transforms[i].GetTransform().GetLocation();

					SpawnDeathFX(DeathLocation);

					UMassProxyPoolSubsystem* Pool = GetWorld()->GetSubsystem<UMassProxyPoolSubsystem>();
					if (IsValid(Pool) == true)
					{
						Pool->Release(Entity);
					}
					Context.Defer().DestroyEntity(Entity);
				}
			}
		});
}

void UMassBoidsDestructionProcessor::SpawnDeathFX(const FVector& DeathLocation) const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	if (IsValid(ExplosionEffect) == true)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ExplosionEffect,
			DeathLocation,
			FRotator::ZeroRotator,
			EffectScale,
			true,  // Auto Destroy
			true,  // Auto Activate
			ENCPoolMethod::AutoRelease
		);
	}

	if (IsValid(ExplosionSound) == true)
	{
		UGameplayStatics::PlaySoundAtLocation(World, ExplosionSound, DeathLocation);
	}
}
