// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassProxyPoolSubsystem.generated.h"

class ACollisionProxyActor;

/**
 * 
 */
UCLASS()
class PROJECTD_API UMassProxyPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Proxy")
	void InitPool(int32 PoolSize);

	UFUNCTION(BlueprintCallable, Category = "Proxy")
	void ShutdownPool();

	// Acquire Proxy
	ACollisionProxyActor* Acquire(const FMassEntityHandle& Entity);

	// Release Proxy
	void Release(const FMassEntityHandle& Entity);

	// Check Proxy
	ACollisionProxyActor* GetProxy(const FMassEntityHandle& Entity) const;

	bool IsServerWorld() const;

	int64 MakeEntityKey(const FMassEntityHandle& E) const;
public:
	FORCEINLINE int32 GetFreeCount() const { return FreeList.Num(); }
	FORCEINLINE int32 GetTotalCount() const { return Proxies.Num(); }

private:
	ACollisionProxyActor* SpawnOneProxyActor();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Proxy")
	TSubclassOf<ACollisionProxyActor> ProxyActorClass;

	UPROPERTY()
	TArray<TWeakObjectPtr<ACollisionProxyActor>> Proxies;

	UPROPERTY()
	TArray<int32> FreeList;

	TMap<int64, int32> EntityToProxyId;
};
