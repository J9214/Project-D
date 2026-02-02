// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassProxyPoolSubsystem.h"
#include "AI/MassAI/CollisionProxyActor.h"
#include "ProjectD/ProjectD.h"
#include "Components/CapsuleComponent.h"

void UMassProxyPoolSubsystem::Deinitialize()
{
	ShutdownPool();
	Super::Deinitialize();
}

void UMassProxyPoolSubsystem::InitPool(int32 PoolSize)
{
	if (IsServerWorld() == false)
	{
		// Client Not Create Proxy Actors
		return;
	}

	if (Proxies.Num() > 0)
	{
		ShutdownPool();
	}

	if (PoolSize <= 0)
	{
		return;
	}

	Proxies.Reserve(PoolSize);
	FreeList.Reserve(PoolSize);

	for (int32 i = 0; i < PoolSize; ++i)
	{
		ACollisionProxyActor* Proxy = SpawnOneProxyActor();
		if (IsValid(Proxy) == false)
		{
			UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::InitPool - Spawn Failed! Stop Spawn Proxy Actor!!"));
			break;
		}

		Proxies.Add(Proxy);
		FreeList.Add(i);
	}
}

void UMassProxyPoolSubsystem::ShutdownPool()
{
	EntityToProxyId.Empty();

	for (TWeakObjectPtr<ACollisionProxyActor>& WeakProxy : Proxies)
	{
		if (WeakProxy.IsValid())
		{
			WeakProxy->Destroy();
		}
	}
	Proxies.Empty();
	FreeList.Empty();
}

ACollisionProxyActor* UMassProxyPoolSubsystem::Acquire(const FMassEntityHandle& Entity)
{
	if (IsServerWorld() == false ||
		Entity.IsValid() == false)
	{
		return nullptr;
	}

	if (ACollisionProxyActor* Existing = GetProxy(Entity))
	{
		return Existing;
	}

	if (FreeList.Num() == 0)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::Acquire - FreeList Is Empty!!"));
		return nullptr;
	}

	const int32 ProxyId = FreeList.Pop();
	if (Proxies.IsValidIndex(ProxyId) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::Acquire - Is Not Valid Idx!!"));
		return nullptr;
	}

	ACollisionProxyActor* Proxy = Proxies[ProxyId].Get();

	// 혹시 액터가 날아갔으면(레벨 전환/에디터 PIE 등) 재생성
	if (IsValid(Proxy) == false)
	{
		Proxy = SpawnOneProxyActor();
		if (IsValid(Proxy) == false)
		{
			UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::Acquire - Can't Spawn Proxy Actor!!"));
			FreeList.Add(ProxyId);
			return nullptr;
		}
		Proxies[ProxyId] = Proxy;
	}

	Proxy->SetRepresentedEntity(Entity);

	if (UCapsuleComponent* Capsule = Proxy->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// Position is Set By Processor Soon
	Proxy->SetActorLocation(FVector::ZeroVector);

	const int64 Key = MakeEntityKey(Entity);
	EntityToProxyId.Add(Key, ProxyId);

	return Proxy;
}

void UMassProxyPoolSubsystem::Release(const FMassEntityHandle& Entity)
{
	if (IsServerWorld() == false ||
		Entity.IsValid() == false)
	{
		return;
	}

	const int64 Key = MakeEntityKey(Entity);
	int32 ProxyId = INDEX_NONE;

	{
		const int32* FoundId = EntityToProxyId.Find(Key);
		if (FoundId == nullptr)
		{
			UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::Release - Can't Found Entity!!"));
			return;
		}
		ProxyId = *FoundId;
	}

	EntityToProxyId.Remove(Key);

	if (Proxies.IsValidIndex(ProxyId) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::Release - Proxy Id is not Valid!!"));
		return;
	}

	ACollisionProxyActor* Proxy = Proxies[ProxyId].Get();
	if (IsValid(Proxy))
	{
		Proxy->ResetProxy();
	}

	FreeList.Add(ProxyId);
}

ACollisionProxyActor* UMassProxyPoolSubsystem::GetProxy(const FMassEntityHandle& Entity) const
{
	if (IsServerWorld() == false ||
		Entity.IsValid() == false)
	{
		return nullptr;
	}

	const int64 Key = MakeEntityKey(Entity);
	const int32* FoundId = EntityToProxyId.Find(Key);
	if (FoundId == nullptr)
	{
		return nullptr;
	}

	const int32 ProxyId = *FoundId;
	if (Proxies.IsValidIndex(ProxyId) == false)
	{
		return nullptr;
	}

	return Proxies[ProxyId].Get();
}

bool UMassProxyPoolSubsystem::IsServerWorld() const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::IsServerWorld - World Is Not Valid...?"));
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	if (NetMode != NM_DedicatedServer)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::IsServerWorld - Is Not Dedi Server!"));
		return false;
	}

	return true;
}

int64 UMassProxyPoolSubsystem::MakeEntityKey(const FMassEntityHandle& E)
{
	return (static_cast<int64>(E.Index) << 32) | static_cast<int64>(E.SerialNumber);
}

ACollisionProxyActor* UMassProxyPoolSubsystem::SpawnOneProxyActor()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassProxyPoolSubsystem::SpawnOneProxyActor - World Is Not Valid...?"));
		return nullptr;
	}

	UClass* SpawnClass = ProxyActorClass ? *ProxyActorClass : ACollisionProxyActor::StaticClass();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bDeferConstruction = false;
	Params.ObjectFlags |= RF_Transient;

	ACollisionProxyActor* Proxy = World->SpawnActor<ACollisionProxyActor>(SpawnClass, FTransform::Identity, Params);

	if (IsValid(Proxy))
	{
		Proxy->ResetProxy();
		Proxy->SetActorHiddenInGame(true);
	}

	return Proxy;
}
