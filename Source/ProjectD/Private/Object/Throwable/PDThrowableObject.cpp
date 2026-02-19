#include "Object/Throwable/PDThrowableObject.h"

#include "Object/Throwable/PDThrowableDataAsset.h"
#include "Pawn/PDPawnBase.h"

#include "Engine/AssetManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "ProjectD/ProjectD.h"

TSet<FName> APDThrowableObject::PDANameSet;

APDThrowableObject::APDThrowableObject()
{
	PDAType = "Throw";

	StaticMesh->SetSimulatePhysics(false);

	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	Projectile->SetIsReplicated(true);
}

void APDThrowableObject::BeginPlay()
{
	Super::BeginPlay();

	bIsCanInteract = true;
	Projectile->Deactivate();

	if (HasAuthority())
	{
		if (!PDANameSet.Contains(PDAName))
		{
			LoadPDA();
		}
	}
}

void APDThrowableObject::OnInteract_Implementation(AActor* Interactor)
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

void APDThrowableObject::OnEndInteract_Implementation(AActor* Interactor)
{
	bIsCanInteract = false;
	ThrowObject();
}

void APDThrowableObject::DropPhysics(const FVector& DropLocation, const FVector& Impulse)
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);

	StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Execute_OnEndInteract(this, CarrierPawn.Get());
}

bool APDThrowableObject::IsCanInteract(AActor* Interactor)
{
	APDPawnBase* Pawn = Cast<APDPawnBase>(Interactor);

	if (!IsValid(Pawn))
	{
		return false;
	}

	return !Pawn->GetCarriedObject() && bIsCanInteract;
}

void APDThrowableObject::ThrowObject()
{
	UAssetManager* AssetManager = UAssetManager::GetIfValid();
	if (!IsValid(AssetManager))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDThrowableObject::ThrowObject - Invalid Asset Manager!"));
		return;
	}

	UObject* Data = AssetManager->GetPrimaryAssetObject(FPrimaryAssetId(PDAType, PDAName));
	if (!IsValid(Data))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDThrowableObject::ThrowObject - No %s / %s Data!"), *PDAType.ToString(), *PDAName.ToString());
		return;
	}

	UPDThrowableDataAsset* ThrowData = Cast<UPDThrowableDataAsset>(Data);
	if (!IsValid(ThrowData))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDThrowableObject::ThrowObject - Data is not PDThrowableDataAsset!"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(CarrierPawn.Get()->GetController());
	if (!IsValid(PC))
	{
		return;
	}

	FVector CamLocation;
	FRotator CamRotation;
	PC->GetPlayerViewPoint(CamLocation, CamRotation);

	CamRotation.Pitch += 8.0f;
	FVector Forward = CamRotation.Vector();
	FVector UpBoost = FVector::UpVector * ThrowData->AdjZVelocity;

	FVector Direction = (Forward + UpBoost).GetSafeNormal();

	Projectile->SetUpdatedComponent(RootComponent);

	Projectile->InitialSpeed = ThrowData->InitialSpeed;
	Projectile->MaxSpeed = ThrowData->MaxSpeed;
	Projectile->ProjectileGravityScale = ThrowData->GravityScale;
	Projectile->Velocity = Direction * Projectile->InitialSpeed;

	Projectile->Activate();
}

void APDThrowableObject::HandleCarrierChanged()
{
	if (!IsValid(CarrierPawn.Get()))
	{
		return;
	}

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
				StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				SetActorRelativeLocation(FVector::ZeroVector);
				SetActorRelativeRotation(FRotator::ZeroRotator);
			}
		}
	}
}

void APDThrowableObject::LoadPDA()
{
	UAssetManager* AssetManager = UAssetManager::GetIfValid();
	if (!IsValid(AssetManager))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDThrowableObject::LoadPDA - Invalid Asset Manager!"));
		return;
	}

	TSharedPtr<FStreamableHandle> Handle = AssetManager->LoadPrimaryAsset(FPrimaryAssetId(PDAType, PDAName), TArray<FName>());
	if (Handle.IsValid())
	{
		PDANameSet.Add(PDAName);
	}
}
