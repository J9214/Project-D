#include "Object/BallCore.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Pawn/PDPawnBase.h" 
#include "GameState/PDGameStateBase.h"
#include "PlayerState/PDPlayerState.h"

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

	StaticMesh->WakeAllRigidBodies();
	StaticMesh->AddImpulse(Impulse, NAME_None, true);
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
	else
	{
		if (!bIsPlacedInGoal)
		{
			StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

			SetActorEnableCollision(true);
			StaticMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
			StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			StaticMesh->SetSimulatePhysics(true);
			StaticMesh->SetEnableGravity(true);
		}
	}
}