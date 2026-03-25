#include "Object/BallCore.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Pawn/PDPawnBase.h" 
#include "GameState/PDGameStateBase.h"
#include "PlayerState/PDPlayerState.h"
#include "ProjectD/ProjectD.h"
#include "GameMode/PDGameModeBase.h"
#include "Components/SphereComponent.h"

ABallCore::ABallCore()
{
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetCollisionProfileName(TEXT("PhysicsActor"));
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Sphere->SetSimulatePhysics(true);
	Sphere->SetEnableGravity(true);
	Sphere->SetMobility(EComponentMobility::Movable);
	Sphere->SetIsReplicated(true);
	Sphere->BodyInstance.bUseCCD = true;

	SetRootComponent(Sphere);

	SceneRoot->SetupAttachment(Sphere);

	StaticMesh->SetupAttachment(SceneRoot);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMesh->SetSimulatePhysics(false);
	StaticMesh->SetEnableGravity(false);

	bReplicates = true;
	SetReplicateMovement(true);
}

void ABallCore::OnInteract_Implementation(AActor* Interactor)
{
	if (!IsCanInteract(Interactor))
	{
		return;
	}

	if (APDPawnBase* PDPawn = Cast<APDPawnBase>(Interactor))
	{
		PDPawn->Server_PickUpObject(this);
	}
}

void ABallCore::DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection)
{
	Super::DropPhysics(DropLocation, Impulse, InCamDirection);

	if (IsValid(Sphere) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] DropPhysics failed. StaticMesh is invalid."));
		return;
	}

	Sphere->WakeAllRigidBodies();
	Sphere->AddImpulse(Impulse, NAME_None, true);
}

void ABallCore::ResetBallForRound(const FVector& SpawnLocation)
{
	if (HasAuthority() == false)
	{
		return;
	}

	Multicast_ApplyRoundReset(SpawnLocation);
}

void ABallCore::PlaceIntoGoal(USceneComponent* GoalAttachParent)
{
	if (!HasAuthority() || !IsValid(GoalAttachParent))
	{
		return;
	}

	CarrierPawn = nullptr;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	Sphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Sphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	Sphere->SetSimulatePhysics(false);
	Sphere->SetEnableGravity(false);
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Sphere->SetCollisionProfileName(TEXT("NoCollision"));

	AttachToComponent(
		GoalAttachParent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	SceneRoot->SetRelativeLocation(FVector::ZeroVector);
	SceneRoot->SetRelativeRotation(FRotator::ZeroRotator);

	StaticMesh->SetRelativeLocation(FVector::ZeroVector);
	StaticMesh->SetRelativeRotation(FRotator::ZeroRotator);

	bIsPlacedInGoal = true;
	SetActorEnableCollision(false);
}

void ABallCore::HandleCarrierChanged()
{
	if (IsValid(CarrierPawn.Get()))
	{
		if (APDGameStateBase* GS = GetWorld()->GetGameState<APDGameStateBase>())
		{
			if (APlayerState* PS = CarrierPawn->GetPlayerState())
			{
				if (APDPlayerState* MyPS = Cast<APDPlayerState>(PS))
				{
					GS->SetBallHolder(MyPS);

					if (APDGameModeBase* GM = GetWorld()->GetAuthGameMode<APDGameModeBase>())
					{
						GM->HandleBallPickedUp(MyPS, this);
					}
				}
			}
		}

		Sphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Sphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		Sphere->SetSimulatePhysics(false);
		Sphere->SetEnableGravity(false);
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Sphere->SetCollisionProfileName(TEXT("NoCollision"));
		
		SetActorEnableCollision(false);
		
		if (APDPawnBase* PD = Cast<APDPawnBase>(CarrierPawn))
		{
			if (USkeletalMeshComponent* CharacterMesh = PD->GetSkeletalMeshComponent())
			{
				AttachToComponent(
					CharacterMesh,
					FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					PD->BallSocketName
				);
			}
		}
	}
	else
	{
		if (bIsPlacedInGoal)
		{
			return;
		}

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		SetActorEnableCollision(true);
		Sphere->SetCollisionProfileName(TEXT("PhysicsActor"));
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Sphere->SetEnableGravity(true);
		Sphere->SetSimulatePhysics(true);
		Sphere->WakeAllRigidBodies();
	}
}

UPrimitiveComponent* ABallCore::GetPhysicsComponent() const
{
	return Sphere;
}

void ABallCore::ApplyRoundResetLocal(const FVector& SpawnLocation)
{
	if (SpawnLocation == FVector::ZeroVector)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] ApplyRoundResetLocal failed. SpawnLocation is zero vector."));
		return;
	}

	if (IsValid(Sphere) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] ApplyRoundResetLocal failed. StaticMesh is invalid."));
		return;
	}

	CarrierPawn = nullptr;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	Sphere->SetCollisionProfileName(TEXT("PhysicsActor"));
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Sphere->SetEnableGravity(true);
	Sphere->SetSimulatePhysics(true);

	Sphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Sphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	SetActorLocation(SpawnLocation);
	SetActorRotation(FRotator::ZeroRotator);

	Sphere->WakeAllRigidBodies();

	bIsCanInteract = true;
	bIsPlacedInGoal = false;

	UE_LOG(
		LogProjectD,
		Log,
		TEXT("[BallCore] ApplyRoundResetLocal completed. SpawnLocation=(%.2f, %.2f, %.2f)"),
		SpawnLocation.X,
		SpawnLocation.Y,
		SpawnLocation.Z
	);
}

void ABallCore::Multicast_ApplyRoundReset_Implementation(const FVector& SpawnLocation)
{
	ApplyRoundResetLocal(SpawnLocation);
}
