// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassBoidsProcessor.generated.h"

struct FMassBoidsFragment;
struct FTransformFragment;
struct FMassVelocityFragment;

/**
 * 
 */
UCLASS()
class PROJECTD_API UMassBoidsProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
    UMassBoidsProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FVector ComputeSeparation(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const;
    FVector ComputeAlignment(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const;
    FVector ComputeCohesion(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const;
    FVector ComputeBounds(const FVector& MyPos, const FVector& MyVel, const FVector& CenterPos, float Radius, const FMassBoidsFragment& Settings) const;
    FVector ComputeObstacleAvoidance(const FVector& MyPos, const FVector& MyVel, const FMassBoidsFragment& Settings, const UWorld* World) const;

    FVector SteerTowards(const FVector& DesiredDirection, const FVector& CurrentVel, const FMassBoidsFragment& Settings) const;

private:
    FMassEntityQuery EntityQuery;
};
