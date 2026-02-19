#include "AI/MassAI/MassProxySyncProcessor.h"
#include "AI/MassAI/MassProxyPoolSubsystem.h"
#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "AI/MassAI/CollisionProxyActor.h"
#include "MassExecutionContext.h"

UMassProxySyncProcessor::UMassProxySyncProcessor()
	:EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
	bAutoRegisterWithProcessingPhases = true;

	bRequiresGameThreadExecution = true;

	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UMassProxySyncProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	UWorld* World = Owner.GetWorld();
	if (IsValid(World) == true)
	{
		ProxyPoolSubsystem = World->GetSubsystem<UMassProxyPoolSubsystem>();
	}
}

void UMassProxySyncProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassProxySyncProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	if (IsValid(ProxyPoolSubsystem) == false)
	{
		return;
	}

	TimeSinceLastUpdateSeconds += Context.GetDeltaTimeSeconds();
	if (UpdateIntervalSeconds > 0.0f)
	{
		if (TimeSinceLastUpdateSeconds < UpdateIntervalSeconds)
		{
			return;
		}
	}
	TimeSinceLastUpdateSeconds = 0.0f;

	TArray<FMassEntityHandle> ToRelease;
	ToRelease.Reserve(16);

	ProxyPoolSubsystem->ForEachActiveProxy([&](ACollisionProxyActor* Proxy)
		{
			if (IsValid(Proxy) == false)
			{
				return;
			}

			const FMassEntityHandle Entity = Proxy->GetRepresentedEntity();

			if (EntityManager.IsEntityValid(Entity) == false)
			{
				ToRelease.Add(Entity);
				return;
			}

			const FTransformFragment* TransformFrag = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
			if (TransformFrag == nullptr)
			{
				ToRelease.Add(Entity);
				return;
			}

			const FTransform& T = TransformFrag->GetTransform();

			Proxy->SetActorLocation(T.GetLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		});

	for (const FMassEntityHandle& E : ToRelease)
	{
		ProxyPoolSubsystem->Release(E);
	}
}
