// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassBoidsProcessor.h"
#include "MassCommonFragments.h"		// Tranform Fragment
#include "MassMovementFragments.h"		// VelocityFragment
#include "MassExecutionContext.h"
#include "AI/MassAI/MassBoidsFragment.h"
#include "AI/MassAI/MassTargetFragment.h"

UMassBoidsProcessor::UMassBoidsProcessor()
	:EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UMassBoidsProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSharedRequirement<FMassBoidsFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBoidsProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FMassVelocityFragment> Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();
			TConstArrayView<FMassTargetFragment> TargetInfos = Context.GetFragmentView<FMassTargetFragment>();

			const FMassBoidsFragment& Settings = Context.GetSharedFragment<FMassBoidsFragment>();

			const float DT = Context.GetDeltaTimeSeconds();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FTransform& Transform = Transforms[i].GetMutableTransform();
				FVector& Velocity = Velocities[i].Value;
				const FMassTargetFragment& TargetInfo = TargetInfos[i];

				FVector CurrentPos = Transform.GetLocation();
				FVector Acceleration = FVector::ZeroVector;

				if (TargetInfo.IsTargetChase)
				{
					Acceleration += ComputeBounds(CurrentPos, Velocity, TargetInfo.TargetPosition, Settings.BoundsRadius, Settings) * Settings.BoundsWeight;
				}

				FVector SepForce = ComputeSeparation(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += SepForce * Settings.SeparationWeight;

				FVector AliForce = ComputeAlignment(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += AliForce * Settings.AlignmentWeight;

				FVector CohForce = ComputeCohesion(CurrentPos, Velocity, i, Transforms, Velocities, Settings, NumEntities);
				Acceleration += CohForce * Settings.CohesionWeight;

				FVector AvoidForce = ComputeObstacleAvoidance(CurrentPos, Velocity, Settings, GetWorld());
				Acceleration += AvoidForce * Settings.ObstacleAvoidanceWeight;

				Velocity += Acceleration * DT;

				float Speed = Velocity.Size();
				if (FMath::IsNearlyZero(Speed) == false)
				{
					float NewSpeed = FMath::Clamp(Speed, Settings.MinMoveSpeed, Settings.MaxMoveSpeed);
					Velocity = (Velocity / Speed) * NewSpeed;
				}

				Transform.AddToTranslation(Velocity * DT);

				if (Velocity.IsNearlyZero() == false)
				{
					FQuat TargetRotation = Velocity.ToOrientationQuat();
					FQuat CurrentRotation = Transform.GetRotation();
					Transform.SetRotation(FQuat::Slerp(CurrentRotation, TargetRotation, Settings.RotationSpeed * DT));
				}
			}
		});
}

FVector UMassBoidsProcessor::ComputeSeparation(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const
{
	FVector Steering = FVector::ZeroVector;
	int32 Count = 0;

	const float Epsilon = 1.0f;

	for (int32 j = 0; j < NumEntities; ++j)
	{
		if (MyIndex == j)
		{
			continue;
		}

		FVector OtherPos = Transforms[j].GetTransform().GetLocation();
		float DistSq = FVector::DistSquared(MyPos, OtherPos);

		if (DistSq < (Settings.AvoidRadius * Settings.AvoidRadius))
		{
			FVector Diff = MyPos - OtherPos;
			float Scale = 1.0f / (FMath::Max(DistSq, Epsilon));

			Steering += Diff.GetSafeNormal() * Scale;
			Count++;
		}
	}

	if (Count > 0)
	{
		return SteerTowards(Steering, MyVel, Settings);
	}

	return Steering;
}

FVector UMassBoidsProcessor::ComputeAlignment(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const
{
	FVector AvgVel = FVector::ZeroVector;
	int32 Count = 0;

	for (int32 j = 0; j < NumEntities; ++j)
	{
		if (MyIndex == j)
		{
			continue;
		}
		FVector OtherPos = Transforms[j].GetTransform().GetLocation();
		float DistSq = FVector::DistSquared(MyPos, OtherPos);

		if (DistSq < (Settings.DetectionRadius * Settings.DetectionRadius))
		{
			AvgVel += Velocities[j].Value;
			Count++;
		}
	}

	if (Count > 0)
	{
		AvgVel /= (float)Count;
		
		return SteerTowards(AvgVel, MyVel, Settings);
	}

	return FVector::ZeroVector;
}

FVector UMassBoidsProcessor::ComputeCohesion(const FVector& MyPos, const FVector& MyVel, int32 MyIndex, TArrayView<FTransformFragment> Transforms, TArrayView<FMassVelocityFragment> Velocities, const FMassBoidsFragment& Settings, int32 NumEntities) const
{
	FVector CenterOfMass = FVector::ZeroVector;
	int32 Count = 0;

	for (int32 j = 0; j < NumEntities; ++j)
	{
		if (MyIndex == j)
		{
			continue;
		}

		FVector OtherPos = Transforms[j].GetTransform().GetLocation();
		float DistSq = FVector::DistSquared(MyPos, OtherPos);

		if (DistSq < (Settings.DetectionRadius * Settings.DetectionRadius))
		{
			CenterOfMass += OtherPos;
			Count++;
		}
	}

	if (Count > 0)
	{
		CenterOfMass /= (float)Count;

		return SteerTowards(CenterOfMass - MyPos, MyVel, Settings);
	}

	return FVector::ZeroVector;
}

FVector UMassBoidsProcessor::ComputeBounds(const FVector& MyPos, const FVector& MyVel, const FVector& CenterPos, float Radius, const FMassBoidsFragment& Settings) const
{
	FVector Offset = CenterPos - MyPos;
	float DistSq = Offset.SizeSquared();
	float RadiusSq = Radius * Radius;

	if (DistSq > RadiusSq)
	{
		FVector PullForce = Offset.GetSafeNormal();

		FVector TangentForce = FVector::CrossProduct(PullForce, FVector::UpVector);

		FVector FinalDir = (PullForce + (TangentForce * Settings.SpiralFactor)).GetSafeNormal();
		return SteerTowards(FinalDir, MyVel, Settings);
	}

	return FVector::ZeroVector;
}

FVector UMassBoidsProcessor::ComputeObstacleAvoidance(const FVector& MyPos, const FVector& MyVel, const FMassBoidsFragment& Settings, const UWorld* World) const
{
	if (IsValid(World) == false)
	{
		return FVector::ZeroVector;
	}

	FVector Forward = MyVel.GetSafeNormal();
	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}

	FQuat VelocityQuat = Forward.ToOrientationQuat();
	float CheckDistance = Settings.ObstacleCheckDistance;

	FVector TotalAvoidForce = FVector::ZeroVector;
	int32 HitCount = 0;

	struct RayConfig
	{
		FVector Direction;
		float LengthScale;
		float WeightMultiplier;
	};

	const float Angle = Settings.ObstacleAvoidanceDegree;

	int32 TotalRays = Settings.ObstacleAvoidanceLineCount;
	if (TotalRays < 1)
	{
		TotalRays = 1;
	}

	TArray<RayConfig> Rays;
	Rays.Reserve(TotalRays);

	Rays.Add({ FVector::ForwardVector, 1.0f, 2.0f });

	if (TotalRays > 1)
	{
		int32 SideRayCount = TotalRays - 1;

		float AngleStep = 360.0f / (float)SideRayCount;

		FVector BaseConeVec = FRotator(Angle, 0, 0).Vector();

		for (int32 i = 0; i < SideRayCount; ++i)
		{
			float CurrentRoll = i * AngleStep;
			FVector RotatedDir = BaseConeVec.RotateAngleAxis(CurrentRoll, FVector::ForwardVector);
			Rays.Add({ RotatedDir, 0.8f, 1.0f });
		}
	}

	FCollisionQueryParams Params;

	for (const RayConfig& Ray : Rays)
	{
		FVector WorldDir = VelocityQuat.RotateVector(Ray.Direction);

		FVector Start = MyPos;
		FVector End = MyPos + (WorldDir * (CheckDistance * Ray.LengthScale));

		FHitResult Hit;
		bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);

		if (bHit)
		{
			FVector AvoidDir = Hit.ImpactNormal;

			float DistRatio = (Hit.Distance / (CheckDistance * Ray.LengthScale));
			float StrengthByDist = 1.0f - DistRatio;

			TotalAvoidForce += AvoidDir * StrengthByDist * Ray.WeightMultiplier;
			HitCount++;
		}
	}

	if (HitCount > 0)
	{
		return SteerTowards(TotalAvoidForce, MyVel, Settings);
	}

	return FVector::ZeroVector;
}

FVector UMassBoidsProcessor::SteerTowards(const FVector& DesiredDirection, const FVector& CurrentVel, const FMassBoidsFragment& Settings) const
{
	FVector DesiredVelocity = DesiredDirection.GetSafeNormal() * Settings.MaxMoveSpeed;
	FVector Steer = DesiredVelocity - CurrentVel;
	return Steer.GetClampedToMaxSize(Settings.MaxSteerWeight);
}
