#include "Weapon/PDThrowableProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"
#include "Kismet/GameplayStatics.h"

APDThrowableProjectile::APDThrowableProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	bReplicates = true;
	SetReplicateMovement(true);

	Collision = CreateDefaultSubobject<USphereComponent>("Collision");
	RootComponent = Collision;
	Collision->InitSphereRadius(10.f);
	Collision->SetCollisionProfileName("Projectile");
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);

	ProjectileMove = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMove");
	ProjectileMove->bRotationFollowsVelocity = true;
	ProjectileMove->bShouldBounce = true;
	ProjectileMove->Bounciness = 0.25f;
	ProjectileMove->Friction = 0.2f;
	ProjectileMove->ProjectileGravityScale = 1.0f;
}

void APDThrowableProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void APDThrowableProjectile::InitFromData(UDataAsset_Throwable* Data, const FVector& Start, const FVector& Velocity)
{
	ThrowableData = Data;
	SetActorLocation(Start);

	ProjectileMove->Velocity = Velocity;
	ProjectileMove->ProjectileGravityScale = Data ? Data->GravityScale : 1.0f;

	if (HasAuthority())
	{
		const float Fuse = Data ? Data->FuseTime : 3.0f;
		GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &APDThrowableProjectile::Explode, Fuse, false);
	}
}

void APDThrowableProjectile::Explode()
{
	if (!HasAuthority() || !ThrowableData)
	{
		return;
	}
	
	TArray<AActor*> Overlapped;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), Overlapped);
	
	// Apply GE
	
	// SFX/VFX
}
