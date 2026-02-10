#include "Gimmick/Teleporter/PDTeleporter.h"

#include "Components/CapsuleComponent.h"

#include "MoverComponent.h"

#include "ProjectD/ProjectD.h"

APDTeleporter::APDTeleporter()
{
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CapsuleComp->SetupAttachment(RootComponent);

	Destination = CreateDefaultSubobject<USceneComponent>(TEXT("Destination"));
	Destination->SetupAttachment(RootComponent);

	Shape = CapsuleComp;
}

void APDTeleporter::OnInteract_Implementation(AActor* Interactor)
{
	if (!TeleportDest.IsValid())
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDTeleporter::OnInteract - Invalid Destination!"));
		return;
	}

	UMoverComponent* Mover = Interactor->GetComponentByClass<UMoverComponent>();
	if (!IsValid(Mover))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDTeleporter::OnInteract - Interactor has Invalid Mover!"));
		return;
	}

	USceneComponent* UpdatedComp = Mover->GetUpdatedComponent();
	if (!IsValid(UpdatedComp))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDTeleporter::OnInteract - Mover has Invalid Updated Component!"));
		return;
	}

	UpdatedComp->SetWorldLocationAndRotation(
		TeleportDest->Destination->GetComponentLocation(),
		TeleportDest->GetActorQuat(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);
}

void APDTeleporter::OnEndInteract_Implementation(AActor* Interactor)
{
}
