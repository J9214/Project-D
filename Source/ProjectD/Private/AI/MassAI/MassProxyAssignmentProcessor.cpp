// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassProxyAssignmentProcessor.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "AI/MassAI/MassPerceptionSubsystem.h"
#include "Structs/MassPerceptionType.h"
#include "DataAssets/AI/MassAI/DataAsset_MassPerceptionSettings.h"
#include "AI/MassAI/MassProxyPoolSubsystem.h"
#include "AI/MassAI/MassBoidsHealthFragment.h"
#include "AI/MassAI/CollisionProxyActor.h"
#include "Subsystem/PlayerLocSubsystem.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UMassProxyAssignmentProcessor::UMassProxyAssignmentProcessor()
	:EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
	bAutoRegisterWithProcessingPhases = true;

	// Actor Spawn / Move
	bRequiresGameThreadExecution = true;

	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UMassProxyAssignmentProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner,EntityManager);

	UWorld* World = Owner.GetWorld();
	if (IsValid(World) == true)
	{
		ProxyPoolSubsystem = World->GetSubsystem<UMassProxyPoolSubsystem>();
		PlayerLocSubsystem = World->GetSubsystem<UPlayerLocSubsystem>();
		PerceptionSubsystem = World->GetSubsystem<UMassPerceptionSubsystem>();
	}
}

void UMassProxyAssignmentProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassBoidsHealthFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UMassProxyAssignmentProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	if (IsValid(ProxyPoolSubsystem) == false ||
		IsValid(PlayerLocSubsystem) == false ||
		IsValid(PerceptionSubsystem) == false)
	{
		return;
	}

	if (PerceptionSubsystem->IsServerWorld() == false)
	{
		return;
	}

	const FMassPerceptionTuning* Tuning = PerceptionSubsystem->GetTuningOrNull();
	if (Tuning == nullptr)
	{
		return;
	}

	// interval gate
	TimeSinceLastUpdateSeconds += Context.GetDeltaTimeSeconds();
	if (Tuning->UpdateInterval > 0.0f)
	{
		if (TimeSinceLastUpdateSeconds < Tuning->UpdateInterval)
		{
			return;
		}
	}
	TimeSinceLastUpdateSeconds = 0.0f;

	TArray<FVector> PlayerLocs;
	TArray<FVector> ViewOrigins;
	TArray<FVector> ViewDirs;

	PlayerLocSubsystem->GetPlayerLocations(PlayerLocs);
	PlayerLocSubsystem->GetPlayerViewpoints(ViewOrigins, ViewDirs);

	if (PlayerLocs.Num() == 0 && ViewOrigins.Num() == 0)
	{
		ReleaseAllAssigned();
		return;
	}

	const TArray<FMassAimTubeRequest>& AimTubes = PerceptionSubsystem->GetActiveAimTubes();

	const float NearRadiusSq = Tuning->NearRadius * Tuning->NearRadius;
	const float SightMaxDistSq = Tuning->SightMaxDist * Tuning->SightMaxDist;
	const float SightCosHalfAngle = FMath::Cos(FMath::DegreesToRadians(Tuning->SightHalfAngleDeg));

	struct FCandidate
	{
		FMassEntityHandle Entity;
		float MinDistSq = TNumericLimits<float>::Max();
		bool bInCone = false;
		bool bInAimTube = false;
	};

	TArray<FCandidate> Candidates;
	Candidates.Reserve(256);

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ExecContext)
		{
			const TConstArrayView<FTransformFragment> Transforms = ExecContext.GetFragmentView<FTransformFragment>();
			const TConstArrayView<FMassBoidsHealthFragment> Healths = ExecContext.GetFragmentView<FMassBoidsHealthFragment>();

			const int32 Num = ExecContext.GetNumEntities();
			for (int32 i = 0; i < Num; ++i)
			{
				if (Healths[i].Health <= 0.0f)
				{
					continue;
				}

				const FVector Pos = Transforms[i].GetTransform().GetLocation();

				const bool bNear = IsNearAnyPlayer(Pos, PlayerLocs, NearRadiusSq);
				const bool bCone = IsInAnySightCone(Pos, ViewOrigins, ViewDirs, SightMaxDistSq, SightCosHalfAngle);

				bool bAim = false;
				if (AimTubes.Num() > 0)
				{
					for (const FMassAimTubeRequest& Tube : AimTubes)
					{
						if (IsPointInAimTube(Pos, Tube, Tuning->AimTubeRadius) == true)
						{
							bAim = true;
							break;
						}
					}
				}

				if (bNear == true || bCone == true || bAim == true)
				{
					FCandidate C;
					C.Entity = ExecContext.GetEntity(i);
					C.MinDistSq = GetMinDistSqToPlayers(Pos, PlayerLocs);
					C.bInCone = bCone;
					C.bInAimTube = bAim;
					Candidates.Add(C);
				}
			}
		});

	Candidates.Sort([](const FCandidate& A, const FCandidate& B)
		{
			if (A.bInAimTube == true && B.bInAimTube == false)
			{
				return true;
			}
			if (A.bInAimTube == false && B.bInAimTube == true)
			{
				return false;
			}

			if (A.bInCone == true && B.bInCone == false)
			{
				return true;
			}
			if (A.bInCone == false && B.bInCone == true)
			{
				return false;
			}

			return A.MinDistSq < B.MinDistSq;
		});

	TArray<FMassEntityHandle> DesiredEntities;
	DesiredEntities.Reserve(Candidates.Num());

	for (const FCandidate& C : Candidates)
	{
		DesiredEntities.Add(C.Entity);
	}

	ApplyMaxDesiredEntities(Tuning->MaxDesiredEntities, DesiredEntities);

	// Wanted set
	TSet<FMassEntityHandle> Wanted;
	Wanted.Reserve(DesiredEntities.Num());

	for (const FMassEntityHandle& E : DesiredEntities)
	{
		Wanted.Add(E);
	}

	// Release
	for (const FMassEntityHandle& Prev : CurrentlyAssignedEntities)
	{
		const bool bStillWanted = Wanted.Contains(Prev);
		if (bStillWanted == false)
		{
			ProxyPoolSubsystem->Release(Prev);
		}
	}

	// Acquire
	TSet<FMassEntityHandle> NewAssigned;
	NewAssigned.Reserve(Wanted.Num());

	for (const FMassEntityHandle& W : Wanted)
	{
		ACollisionProxyActor* ProxyActor = ProxyPoolSubsystem->Acquire(W);
		if (IsValid(ProxyActor) == true)
		{
			NewAssigned.Add(W);
		}
	}

	CurrentlyAssignedEntities = MoveTemp(NewAssigned);
}

float UMassProxyAssignmentProcessor::GetMinDistSqToPlayers(const FVector& Pos, const TArray<FVector>& PlayerLocs)
{
	float MinDistSq = TNumericLimits<float>::Max();

	for (const FVector& PlayerPos : PlayerLocs)
	{
		const float DistSq = FVector::DistSquared(Pos, PlayerPos);
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
		}
	}

	return MinDistSq;
}


bool UMassProxyAssignmentProcessor::IsNearAnyPlayer(const FVector& Pos, const TArray<FVector>& PlayerLocs, float NearRadiusSq)
{
	for (const FVector& PlayerPos : PlayerLocs)
	{
		const float DistSq = FVector::DistSquared(Pos, PlayerPos);
		if (DistSq <= NearRadiusSq)
		{
			return true;
		}
	}

	return false;
}

bool UMassProxyAssignmentProcessor::IsInAnySightCone(const FVector& Pos, const TArray<FVector>& ViewOrigins, const TArray<FVector>& ViewDirs, float SightMaxDistSq, float SightCosHalfAngle)
{
	const int32 Count = FMath::Min(ViewOrigins.Num(), ViewDirs.Num());

	for (int32 i = 0; i < Count; ++i)
	{
		const FVector Origin = ViewOrigins[i];
		const FVector ViewDir = ViewDirs[i].GetSafeNormal();

		const FVector ToTarget = (Pos - Origin);
		const float DistSq = ToTarget.SizeSquared();

		if (DistSq <= SightMaxDistSq)
		{
			const float Dist = FMath::Sqrt(DistSq);
			if (Dist > KINDA_SMALL_NUMBER)
			{
				const FVector ToTargetDir = ToTarget / Dist;
				const float Dot = FVector::DotProduct(ViewDir, ToTargetDir);

				if (Dot >= SightCosHalfAngle)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool UMassProxyAssignmentProcessor::IsPointInAimTube(const FVector& Point, const FMassAimTubeRequest& Tube, float TubeRadius)
{
	const FVector Origin = Tube.Origin;
	const FVector Dir = Tube.Dir.GetSafeNormal();
	const float Length = Tube.WeaponLength;

	if (Length <= 0.0f)
	{
		return false;
	}

	const FVector End = Origin + (Dir * Length);

	const FVector Segment = (End - Origin);
	const float SegLenSq = Segment.SizeSquared();
	if (SegLenSq <= KINDA_SMALL_NUMBER)
	{
		return (FVector::DistSquared(Point, Origin) <= (TubeRadius * TubeRadius));
	}

	const float T = FMath::Clamp(FVector::DotProduct(Point - Origin, Segment) / SegLenSq, 0.0f, 1.0f);
	const FVector Closest = Origin + Segment * T;

	const float DistSq = FVector::DistSquared(Point, Closest);
	return (DistSq <= (TubeRadius * TubeRadius));
}

void UMassProxyAssignmentProcessor::ApplyMaxDesiredEntities(int32 MaxDesiredEntities, TArray<FMassEntityHandle>& InOutDesired)
{
	if (MaxDesiredEntities <= 0)
	{
		return;
	}

	if (InOutDesired.Num() <= MaxDesiredEntities)
	{
		return;
	}

	InOutDesired.SetNum(MaxDesiredEntities);
}

void UMassProxyAssignmentProcessor::ReleaseAllAssigned()
{
	if (IsValid(ProxyPoolSubsystem) == false)
	{
		CurrentlyAssignedEntities.Reset();
		return;
	}

	for (const FMassEntityHandle& Entity : CurrentlyAssignedEntities)
	{
		ProxyPoolSubsystem->Release(Entity);
	}

	CurrentlyAssignedEntities.Reset();
}
