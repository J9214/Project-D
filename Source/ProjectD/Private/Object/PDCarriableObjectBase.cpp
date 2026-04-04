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
	DOREPLIFETIME(APDCarriableObjectBase, bIsPlacedInGoal);
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

	UPrimitiveComponent* PhysicsComp = GetPhysicsComponent();
	if (!IsValid(PhysicsComp))
	{
		return;
	}
	
	CarrierPawn = nullptr;
	
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);
	
	PhysicsComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	PhysicsComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PhysicsComp->SetSimulatePhysics(true);
	PhysicsComp->SetEnableGravity(true);
	PhysicsComp->WakeAllRigidBodies();
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

void APDCarriableObjectBase::SetPlacedInGoal(bool bInGoal)
{
	if (HasAuthority())
	{
		bIsPlacedInGoal = bInGoal;
		OnRep_IsPlacedInGoal(); 
	}
}

UPrimitiveComponent* APDCarriableObjectBase::GetPhysicsComponent() const
{
	return StaticMesh;
}

void APDCarriableObjectBase::OnRep_IsPlacedInGoal()
{
	UPrimitiveComponent* PhysicsComp = GetPhysicsComponent();
	if (!IsValid(PhysicsComp))
	{
		return;
	}
	
	if (bIsPlacedInGoal)
	{
		PhysicsComp->SetSimulatePhysics(false);
		PhysicsComp->SetEnableGravity(false);
		PhysicsComp->SetCollisionProfileName(TEXT("NoCollision"));
		PhysicsComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetActorEnableCollision(false);
	}
	else
	{
		PhysicsComp->SetCollisionProfileName(TEXT("PhysicsActor"));
		PhysicsComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PhysicsComp->SetEnableGravity(true);
		PhysicsComp->SetSimulatePhysics(true);
		SetActorEnableCollision(true);
	}
}
