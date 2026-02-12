#include "Object/BallCore.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Pawn/PDPawnBase.h" 
#include "GameState/PDGameStateBase.h"
#include "PlayerState/PDPlayerState.h"

ABallCore::ABallCore()
{
	bReplicates = true;
	SetReplicateMovement(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

    Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));

    Mesh->BodyInstance.bUseCCD = true; // optional
}

void ABallCore::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABallCore, CarrierPawn);
}

void ABallCore::OnInteract_Implementation(AActor* Interactor)
{
	if (APDPawnBase* PDPawn = Cast<APDPawnBase>(Interactor))
	{
		PDPawn->Server_PickUpBall(this);
	}
}

void ABallCore::SetCarrier(APawn* NewCarrier)
{
	if (!HasAuthority()) 
	{
		return;
	}

	CarrierPawn = NewCarrier;
	HandleCarrierChanged();
	if (NewCarrier)
	{
		APDGameStateBase* GS = GetWorld()->GetGameState<APDGameStateBase>();
		if (GS)
		{
			APlayerState* PS = NewCarrier->GetPlayerState();
			if (PS)
			{
				APDPlayerState* MyPS = Cast<APDPlayerState>(PS);
				if (MyPS)
				{
					GS->SetBallHolder(MyPS);
				}
			}
		}
	}
}

void ABallCore::ClearCarrier()
{
	if (!HasAuthority())
	{
		return;
	}

	SetCarrier(nullptr);
}

void ABallCore::DropPhysics(const FVector& DropLocation, const FVector& Impulse)
{
	if (!HasAuthority()) 
	{
		return;
	}

	CarrierPawn = nullptr;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);

	SetActorEnableCollision(true);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); 
	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(true);
	Mesh->WakeAllRigidBodies();
	Mesh->AddImpulse(Impulse, NAME_None, true);
}

void ABallCore::OnRep_CarrierPawn()
{
	HandleCarrierChanged();
}

void ABallCore::HandleCarrierChanged()
{
    if (CarrierPawn)
    {
        Mesh->SetSimulatePhysics(false);
        Mesh->SetEnableGravity(false);
        SetActorEnableCollision(false); 
        Mesh->SetCollisionProfileName(TEXT("NoCollision"));

        if (APDPawnBase* PD = Cast<APDPawnBase>(CarrierPawn))
        {
            USkeletalMeshComponent* CharacterMesh = PD->GetSkeletalMeshComponent();
            FName SocketName = PD->BallSocketName;

            if (CharacterMesh)
            {
                bool bAttached = AttachToComponent(
                    CharacterMesh,
                    FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                    SocketName
                );

                if (bAttached)
                {
                    SetActorRelativeLocation(FVector::ZeroVector);
                    SetActorRelativeRotation(FRotator::ZeroRotator);
                }
            }
        }
    }
}