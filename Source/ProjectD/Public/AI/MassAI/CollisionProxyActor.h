// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "CollisionProxyActor.generated.h"

class UCapsuleComponent;

UCLASS()
class PROJECTD_API ACollisionProxyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACollisionProxyActor();

	void SetRepresentedEntity(const FMassEntityHandle& InEntity);
	void ResetProxy();
	
	FORCEINLINE const FMassEntityHandle& GetRepresentedEntity() const { return RepresentedEntity; }
	FORCEINLINE int64 GetRepresentedEntityKey() const { return RepresentedEntityKey; }
	FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return Capsule; }

private:
	static int64 MakeEntityKey(const FMassEntityHandle& E);

private:
	UPROPERTY(VisibleAnywhere, Category = "Proxy")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(EditAnywhere, Category = "Proxy")
	float CapsuleRadius;

	FMassEntityHandle RepresentedEntity;
	int64 RepresentedEntityKey = 0;
};
