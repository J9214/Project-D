#include "Object/Throwable/PDThrowableObject.h"

#include "Explosion/PDExplosionActor.h"
#include "Object/Throwable/PDThrowableDataAsset.h"
#include "Pawn/PDPawnBase.h"

#include "Engine/AssetManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "ProjectD/ProjectD.h"

TSet<FName> APDThrowableObject::PDANameSet;

APDThrowableObject::APDThrowableObject()
{
	PDAType = "Throw";
	PDAExplosionType = "Explosion";

	StaticMesh->SetSimulatePhysics(false);

	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	Projectile->SetIsReplicated(true);
	Projectile->bSweepCollision = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(Capsule);
	StaticMesh->SetupAttachment(Capsule);
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
		StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PDPawn->Server_PickUpObject(this);

		Multicast_SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void APDThrowableObject::OnEndInteract_Implementation(AActor* Interactor)
{
	bIsCanInteract = false;
	ThrowObject();
}

void APDThrowableObject::DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection)
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);

	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Capsule->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	CamDirection = InCamDirection;

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
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
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

	FVector UpBoost = FVector::UpVector * ThrowData->AdjZVelocity;

	FVector Direction = (CamDirection + UpBoost).GetSafeNormal();

	Projectile->SetUpdatedComponent(Capsule);

	Projectile->InitialSpeed = ThrowData->InitialSpeed;
	Projectile->MaxSpeed = ThrowData->MaxSpeed;
	Projectile->ProjectileGravityScale = ThrowData->GravityScale;
	Projectile->Velocity = Direction * Projectile->InitialSpeed;

	Projectile->Activate();

	if (HasAuthority() && !Capsule->OnComponentHit.IsBound())
	{
		Capsule->OnComponentHit.AddDynamic(this, &APDThrowableObject::HandleHit);

		Capsule->SetNotifyRigidBodyCollision(true);
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToAllChannels(ECR_Block);
		Capsule->SetCollisionObjectType(ECC_WorldDynamic);
	}
}

void APDThrowableObject::Explode(const FVector& HitLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(ExplosionClass))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDThrowableObject::Explode - Invalid ExplosionClass!"));
		return;
	}

	APDExplosionActor* Explosion = GetWorld()->SpawnActor<APDExplosionActor>(ExplosionClass, GetActorLocation(), FRotator::ZeroRotator);
	if (!IsValid(Explosion))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDThrowableObject::Explode - Spawn Explosion Fail!"));
		return;
	}

	Explosion->InitExplosion(PDAExplosionName, PDAExplosionType);
}

void APDThrowableObject::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (Capsule->OnComponentHit.IsBound())
	{
		Capsule->OnComponentHit.RemoveDynamic(this, &APDThrowableObject::HandleHit);
	}

	Super::EndPlay(EndPlayReason);
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
				SetActorRelativeLocation(FVector::ZeroVector);
				SetActorRelativeRotation(FRotator::ZeroRotator);
			}
		}
	}
}

void APDThrowableObject::HandleHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	Explode(Hit.Location);

	Destroy();
}

void APDThrowableObject::Multicast_SetCollisionEnabled_Implementation(ECollisionEnabled::Type InType)
{
	Capsule->SetCollisionEnabled(InType);
	StaticMesh->SetCollisionEnabled(InType);
}

void APDThrowableObject::LoadPDA()
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
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
