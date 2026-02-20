#include "TargetActor/TA_ThrowableTrajectory.h"
#include "Pawn/PDPawnBase.h"
#include "Abilities/GameplayAbility.h"
#include "Components/SplineComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"
#include "Weapon/FPDTargetData_Throwable.h"

ATA_ThrowableTrajectory::ATA_ThrowableTrajectory()
{
	PrimaryActorTick.bCanEverTick = true;
	bDestroyOnConfirmation = true;

	PreviewSpline = CreateDefaultSubobject<USplineComponent>("Spline");
	RootComponent = PreviewSpline;
	
	bReplicates = false;
}

void ATA_ThrowableTrajectory::SetThrowableData(const UDataAsset_Throwable* InData)
{
	ThrowableData = InData;
}

void ATA_ThrowableTrajectory::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	
	SourcePawn = Cast<APDPawnBase>(Ability->GetAvatarActorFromActorInfo());
}

void ATA_ThrowableTrajectory::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!SourcePawn || !SourcePawn->IsLocallyControlled() || !ThrowableData)
	{
		return;
	}
	
	UpdatePreview();
}

void ATA_ThrowableTrajectory::ConfirmTargetingAndContinue()
{
	if (!SourcePawn || !ThrowableData)
	{
		CancelTargeting();
		return;
	}
	
	if (!SourcePawn->IsLocallyControlled())
	{
		return;
	}
	
	const FVector Start = ComputeStartLocation();
	const FVector Direction = ComputeThrowDirection();
	const float Speed = ThrowableData->MaxThrowSpeed;
	const FVector Velocity = Direction * Speed;
	
	FPDTargetData_Throwable* NewTD = new FPDTargetData_Throwable();
	NewTD->StartLocation= Start;
	NewTD->InitialVelocity= Velocity;
	NewTD->FuseTime = ThrowableData->FuseTime;
	NewTD->GravityScale = ThrowableData->GravityScale;

	FGameplayAbilityTargetDataHandle Handle;
	Handle.Add(NewTD);

	TargetDataReadyDelegate.Broadcast(Handle);
}

void ATA_ThrowableTrajectory::CancelTargeting()
{
	ClearPreview();
	
	Super::CancelTargeting();
}

FVector ATA_ThrowableTrajectory::ComputeStartLocation() const
{
	if (!SourcePawn)
	{
		return FVector::ZeroVector;
	}
	
	if (USkeletalMeshComponent* Mesh = SourcePawn->GetSkeletalMeshComponent())
	{
		if (Mesh->DoesSocketExist(HandSocketName))
		{
			return Mesh->GetSocketLocation(HandSocketName);
		}
	}
	
	return SourcePawn->GetActorLocation() + FVector(0,0,50.f);
}

FVector ATA_ThrowableTrajectory::ComputeThrowDirection() const
{
	if (!SourcePawn)
	{
		return FVector::ForwardVector;
	}
	
	if (UCameraComponent* Camera = SourcePawn->FindComponentByClass<UCameraComponent>())
	{
		FRotator Rot = Camera->GetComponentRotation();
		Rot.Pitch += 10.f;
		return Rot.Vector();
	}

	return SourcePawn->GetActorForwardVector();
}

void ATA_ThrowableTrajectory::UpdatePreview()
{
	const FVector Start = ComputeStartLocation();
	const FVector Direction = ComputeThrowDirection();
	const float Speed = ThrowableData->MaxThrowSpeed;
	const FVector Velocity = Direction * Speed;
	
	FPredictProjectilePathParams Params;
	Params.ActorsToIgnore.Add(SourcePawn);
	
	Params.StartLocation = Start;
	Params.LaunchVelocity = Velocity;
	Params.ProjectileRadius = 5.f;
	Params.bTraceWithCollision = true;
	Params.TraceChannel = ECC_Visibility;
	
	Params.MaxSimTime = ThrowableData->PredictSimTime;
	Params.SimFrequency = (ThrowableData->PredictStep > 0.f) ? (1.f / ThrowableData->PredictStep) : 20.f;
	Params.OverrideGravityZ = GetWorld()->GetGravityZ() * ThrowableData->GravityScale;
	
	FPredictProjectilePathResult Result;
	const bool bPredict = UGameplayStatics::PredictProjectilePath(this, Params, Result);
	
	PreviewSpline->ClearSplinePoints(false);
	
	if (bPredict)
	{
		for (int32 i=1; i<Result.PathData.Num(); ++i)
		{
			DrawDebugLine(GetWorld(), Result.PathData[i-1].Location, Result.PathData[i].Location,
						  FColor::Green, false, 0.f, 0, 2.f);
		}
		
		for (const FPredictProjectilePathPointData& P : Result.PathData)
		{
			PreviewSpline->AddSplinePoint(P.Location, ESplineCoordinateSpace::World, false);
		}
	}
	
	PreviewSpline->UpdateSpline();
}

void ATA_ThrowableTrajectory::ClearPreview()
{
	if (PreviewSpline)
	{
		PreviewSpline->ClearSplinePoints(true);
	}
}

FGameplayAbilityTargetDataHandle ATA_ThrowableTrajectory::MakeTargetData() const
{
	FGameplayAbilityTargetDataHandle Handle;

	if (!SourcePawn || !ThrowableData)
	{
		return Handle;
	}

	const FVector Start = ComputeStartLocation();
	const FVector Direction = ComputeThrowDirection();
	const float Speed = ThrowableData->MaxThrowSpeed;
	const FVector Velocity = Direction * Speed;

	FPDTargetData_Throwable* NewTD = new FPDTargetData_Throwable();
	NewTD->StartLocation = Start;
	NewTD->InitialVelocity = Velocity;
	NewTD->FuseTime = ThrowableData->FuseTime;
	NewTD->GravityScale = ThrowableData->GravityScale;

	Handle.Add(NewTD);
	
	return Handle;
}
