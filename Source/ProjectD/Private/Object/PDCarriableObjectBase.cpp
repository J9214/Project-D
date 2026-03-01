#include "Object/PDCarriableObjectBase.h"

#include "Pawn/PDPawnBase.h"

#include "Net/UnrealNetwork.h"

APDCarriableObjectBase::APDCarriableObjectBase()
{
	bReplicates = true;
	SetReplicateMovement(true);

	StaticMesh->SetMobility(EComponentMobility::Movable);
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	StaticMesh->BodyInstance.bUseCCD = true;
}

void APDCarriableObjectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APDCarriableObjectBase, CarrierPawn);
}

void APDCarriableObjectBase::SetCarrier(APawn* NewCarrier)
{
	if (!HasAuthority())
	{
		return;
	}

	CarrierPawn = MakeWeakObjectPtr(NewCarrier);
	HandleCarrierChanged();
}

void APDCarriableObjectBase::ClearCarrier()
{
	if (!HasAuthority())
	{
		return;
	}

	SetCarrier(nullptr);
}

void APDCarriableObjectBase::DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection)
{
	if (!HasAuthority())
	{
		return;
	}

	CarrierPawn = nullptr;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);

	SetActorEnableCollision(true);
	StaticMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->SetEnableGravity(true);
}

void APDCarriableObjectBase::OnRep_CarrierPawn()
{
	HandleCarrierChanged();
}

bool APDCarriableObjectBase::IsCanInteract(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return false;
	}

	APDPawnBase* PDPawn = Cast<APDPawnBase>(Interactor);
	if (!IsValid(PDPawn))
	{
		return false;
	}

	return !PDPawn->GetCarriedObject();
}
