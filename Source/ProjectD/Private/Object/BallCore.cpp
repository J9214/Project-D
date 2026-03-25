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
	SetRootComponent(Sphere);
	StaticMesh->SetupAttachment(Sphere);
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMesh->SetSimulatePhysics(false);
	SceneRoot->SetupAttachment(Sphere);

	SetActorEnableCollision(true);
	Sphere->SetCollisionProfileName(TEXT("PhysicsActor"));
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Sphere->SetSimulatePhysics(true);
	Sphere->SetEnableGravity(true);

	Sphere->SetIsReplicated(true);

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

void ABallCore::HandleCarrierChanged()
{
	if (IsValid(CarrierPawn.Get()))
	{
		APDGameStateBase* GS = GetWorld()->GetGameState<APDGameStateBase>();
		if (GS)
		{
			APlayerState* PS = CarrierPawn->GetPlayerState();
			if (PS)
			{
				APDPlayerState* MyPS = Cast<APDPlayerState>(PS);
				if (MyPS)
				{
					GS->SetBallHolder(MyPS);

					APDGameModeBase* GM = GetWorld()->GetAuthGameMode<APDGameModeBase>();
					if (IsValid(GM) == true)
					{
						GM->HandleBallPickedUp(MyPS, this);
					}
				}
			}
		}

		Sphere->SetSimulatePhysics(false);
		Sphere->SetEnableGravity(false);
		SetActorEnableCollision(false);
		Sphere->SetCollisionProfileName(TEXT("NoCollision"));

		if (APDPawnBase* PD = Cast<APDPawnBase>(CarrierPawn))
		{
			USkeletalMeshComponent* CharacterMesh = PD->GetSkeletalMeshComponent();
			FName SocketName = PD->BallSocketName;

			if (CharacterMesh)
			{
				bool bAttached = Sphere->AttachToComponent(
					CharacterMesh,
					FAttachmentTransformRules::KeepWorldTransform,
					SocketName
				);

				if (bAttached)
				{
					Sphere->SetRelativeLocation(FVector::ZeroVector);
					Sphere->SetRelativeRotation(FRotator::ZeroRotator);
				}
			}
		}
	}
	else
	{
		if (!bIsPlacedInGoal)
		{
			Sphere->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

			SetActorEnableCollision(true);
			Sphere->SetCollisionProfileName(TEXT("PhysicsActor"));
			Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Sphere->SetSimulatePhysics(true);
			Sphere->SetEnableGravity(true);
		}
	}
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
