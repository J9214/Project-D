// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
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
	FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return Capsule; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Proxy")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(EditAnywhere, Category = "Proxy")
	float CapsuleRadius;

	UPROPERTY(EditAnywhere, Category = "Proxy")
	float CapsuleHalfHeight;

private:
	FMassEntityHandle RepresentedEntity;
};
