#include "Weapon/PDThrowableSmokeArea.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"

APDThrowableSmokeArea::APDThrowableSmokeArea()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	bReplicates = true;
	SetReplicateMovement(false);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = Collision;

	Collision->InitSphereRadius(450.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APDThrowableSmokeArea::InitFromData(AActor* InOwnerActor, UDataAsset_Throwable* InData)
{
	OwnerActor = InOwnerActor;
	Data = InData;

	const float Radius = (Data ? Data->SmokeRadius : 450.f);
	Collision->SetSphereRadius(Radius);
}

void APDThrowableSmokeArea::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnSmokeVFX();
	}

	if (!HasAuthority() || !Data || !OwnerActor)
	{
		return;
	}

	const float Duration = Data->SmokeDuration > 0.f ? Data->SmokeDuration : 6.0f;
	GetWorldTimerManager().SetTimer(EndTimerHandle, this, &APDThrowableSmokeArea::EndArea, Duration, false);
}

void APDThrowableSmokeArea::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupSmokeVFX();

	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(EndTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void APDThrowableSmokeArea::EndArea()
{
	if (HasAuthority())
	{
		CleanupSmokeVFX();
		Destroy();
	}
}

void APDThrowableSmokeArea::SpawnSmokeVFX()
{
	if (SpawnedSmokeVFX || !SmokeLoopVFX)
	{
		return;
	}

	SpawnedSmokeVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SmokeLoopVFX,
		RootComponent,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		false,
		true
	);

	if (!SpawnedSmokeVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn smoke VFX"));
		return;
	}
}

void APDThrowableSmokeArea::CleanupSmokeVFX()
{
	if (SpawnedSmokeVFX)
	{
		SpawnedSmokeVFX->Deactivate();
		SpawnedSmokeVFX->DestroyComponent();
		SpawnedSmokeVFX = nullptr;
	}
}