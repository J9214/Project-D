#include "Components/InteractionComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Gimmick/PDDirectlyInteractGimmickBase.h"
#include "Gimmick/PDInteractableObject.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (IsValid(OwnerActor) && OwnerActor->HasAuthority())
	{
		BuildInteractionCapsule();
	}

	RefreshInteractState();
}

void UInteractionComponent::BuildInteractionCapsule()
{
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor) || !OwnerActor->HasAuthority())
	{
		return;
	}

	if (IsValid(InteractionCapsule))
	{
		return;
	}

	InteractionCapsule = NewObject<UCapsuleComponent>(OwnerActor, TEXT("InteractionCapsule"));
	if (!IsValid(InteractionCapsule))
	{
		return;
	}

	InteractionCapsule->SetCapsuleRadius(InteractionRadius);
	InteractionCapsule->SetCapsuleHalfHeight(InteractionLength * 0.5f);
	InteractionCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCapsule->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCapsule->SetCollisionResponseToAllChannels(ECR_Overlap);
	InteractionCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	InteractionCapsule->SetGenerateOverlapEvents(true);

	USceneComponent* AttachTarget = OwnerActor->FindComponentByClass<UCameraComponent>();
	if (!IsValid(AttachTarget))
	{
		AttachTarget = OwnerActor->GetRootComponent();
	}

	if (IsValid(AttachTarget))
	{
		InteractionCapsule->SetupAttachment(AttachTarget);
	}

	OwnerActor->AddInstanceComponent(InteractionCapsule);
	InteractionCapsule->RegisterComponent();

	InteractionCapsule->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	InteractionCapsule->SetRelativeLocation(FVector(InteractionLength * 0.5f, 0.f, 0.f));

	InteractionCapsule->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleBeginOverlap);
	InteractionCapsule->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleEndOverlap);
}

void UInteractionComponent::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}

	if (APDDirectlyInteractGimmickBase* InteractTarget = Cast<APDDirectlyInteractGimmickBase>(OtherActor))
	{
		OverlappedTargets.Add(InteractTarget);
		RefreshInteractState();
		UE_LOG(
			LogTemp,
			Log,
			TEXT("Interaction overlap detected. Owner=%s Target=%s CanInteract=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(OtherActor),
			bCanInteract ? TEXT("true") : TEXT("false")
		);
	}
}

void UInteractionComponent::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	if (APDDirectlyInteractGimmickBase* InteractTarget = Cast<APDDirectlyInteractGimmickBase>(OtherActor))
	{
		OverlappedTargets.Remove(InteractTarget);
		RefreshInteractState();
	}
}

void UInteractionComponent::RefreshInteractState()
{
	APDDirectlyInteractGimmickBase* FoundTarget = nullptr;

	for (const TWeakObjectPtr<APDDirectlyInteractGimmickBase>& Target : OverlappedTargets)
	{
		if (Target.IsValid())
		{
			FoundTarget = Target.Get();
			break;
		}
	}

	CurrentInteractTarget = FoundTarget;
	bCanInteract = CurrentInteractTarget.IsValid();
}

void UInteractionComponent::TryInteract()
{
	if (!CurrentInteractTarget.IsValid())
	{
		return;
	}

	AActor* Target = CurrentInteractTarget.Get();
	AActor* Interactor = GetOwner();
	if (!IsValid(Target) || !IsValid(Interactor))
	{
		return;
	}

	if (Target->GetClass()->ImplementsInterface(UPDInteractableObject::StaticClass()))
	{
		IPDInteractableObject::Execute_OnInteract(Target, Interactor);
	}
}

void UInteractionComponent::EndInteract()
{
	if (!CurrentInteractTarget.IsValid())
	{
		return;
	}

	AActor* Target = CurrentInteractTarget.Get();
	AActor* Interactor = GetOwner();
	if (!IsValid(Target) || !IsValid(Interactor))
	{
		return;
	}

	if (Target->GetClass()->ImplementsInterface(UPDInteractableObject::StaticClass()))
	{
		IPDInteractableObject::Execute_OnEndInteract(Target, Interactor);
	}
}

