#include "Object/BallCore.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Pawn/PDPawnBase.h" 
#include "GameState/PDGameStateBase.h"
#include "PlayerState/PDPlayerState.h"
#include "ProjectD/ProjectD.h"
#include "GameMode/PDGameModeBase.h"

ABallCore::ABallCore()
{
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

	if (IsValid(StaticMesh) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] DropPhysics failed. StaticMesh is invalid."));
		return;
	}

	StaticMesh->WakeAllRigidBodies();
	StaticMesh->AddImpulse(Impulse, NAME_None, true);
}

void ABallCore::ResetBallForRound(const FVector& SpawnLocation)
{
	if (SpawnLocation == FVector::ZeroVector)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] ResetBallForRound failed. SpawnLocation is zero vector."));
		return;
	}

	if (IsValid(StaticMesh) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] ResetBallForRound failed. StaticMesh is invalid."));
		return;
	}

	if (IsValid(CarrierPawn.Get()) == true)
	{
		ClearCarrier();
	}

	StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	StaticMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	StaticMesh->SetEnableGravity(true);
	StaticMesh->SetSimulatePhysics(true);

	StaticMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	StaticMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	SetActorLocation(SpawnLocation);
	SetActorRotation(FRotator::ZeroRotator);

	StaticMesh->WakeAllRigidBodies();

	UE_LOG(
		LogProjectD,
		Log,
		TEXT("[BallCore] ResetBallForRound completed. SpawnLocation=(%.2f, %.2f, %.2f)"),
		SpawnLocation.X,
		SpawnLocation.Y,
		SpawnLocation.Z
	);
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

		StaticMesh->SetSimulatePhysics(false);
		StaticMesh->SetEnableGravity(false);
		SetActorEnableCollision(false);
		StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));

		if (APDPawnBase* PD = Cast<APDPawnBase>(CarrierPawn))
		{
			USkeletalMeshComponent* CharacterMesh = PD->GetSkeletalMeshComponent();
			FName SocketName = PD->BallSocketName;

			if (CharacterMesh)
			{
				bool bAttached = StaticMesh->AttachToComponent(
					CharacterMesh,
					FAttachmentTransformRules::KeepWorldTransform,
					SocketName
				);

				if (bAttached)
				{
					StaticMesh->SetRelativeLocation(FVector::ZeroVector);
					StaticMesh->SetRelativeRotation(FRotator::ZeroRotator);
				}
			}
		}
	}
}