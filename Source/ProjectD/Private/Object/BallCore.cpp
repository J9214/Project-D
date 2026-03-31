#include "Object/BallCore.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Pawn/PDPawnBase.h" 
#include "GameState/PDGameStateBase.h"
#include "PlayerState/PDPlayerState.h"
#include "ProjectD/ProjectD.h"
#include "GameMode/PDGameModeBase.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/Ingame/ObjectInfo.h"
#include "UI/PDTeamColorFunctionLibrary.h"
#include <Kismet/GameplayStatics.h>

namespace
{
ETeamType ResolvePawnTeamID(const APawn* Pawn)
{
	const APDPlayerState* PlayerState = IsValid(Pawn) ? Pawn->GetPlayerState<APDPlayerState>() : nullptr;
	return IsValid(PlayerState) ? PlayerState->GetTeamID() : ETeamType::None;
}

ETeamType ResolveLocalViewerTeamID(const AActor* ContextActor)
{
	if (IsValid(ContextActor) == false)
	{
		return ETeamType::None;
	}

	const UWorld* World = ContextActor->GetWorld();
	if (World == nullptr)
	{
		return ETeamType::None;
	}

	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	const APDPlayerState* LocalPlayerState = IsValid(PlayerController) ? PlayerController->GetPlayerState<APDPlayerState>() : nullptr;
	return IsValid(LocalPlayerState) ? LocalPlayerState->GetTeamID() : ETeamType::None;
}
}

ABallCore::ABallCore()
{
	PrimaryActorTick.bCanEverTick = true;
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetCollisionProfileName(TEXT("PhysicsActor"));
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Sphere->SetSimulatePhysics(true);
	Sphere->SetEnableGravity(true);
	Sphere->SetMobility(EComponentMobility::Movable);
	Sphere->SetIsReplicated(true);
	Sphere->BodyInstance.bUseCCD = true;

	SetRootComponent(Sphere);

	SceneRoot->SetupAttachment(Sphere);

	StaticMesh->SetupAttachment(SceneRoot);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMesh->SetSimulatePhysics(false);
	StaticMesh->SetEnableGravity(false);

	bReplicates = true;
	SetReplicateMovement(true);

	BallWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("BallWidget"));
	if (GetRootComponent())
	{
		BallWidget->SetupAttachment(StaticMesh);
	}

	BallWidget->SetWidgetSpace(EWidgetSpace::Screen); 
	BallWidget->SetDrawAtDesiredSize(true);
}

void ABallCore::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABallCore, ObjectInfoTeamID);
}

void ABallCore::BeginPlay()
{
	Super::BeginPlay();

	if (BallWidget)
	{
		CachedInfoWidget = Cast<UObjectInfo>(BallWidget->GetUserWidgetObject());

		if (!CachedInfoWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("BallWidget's UserWidget is not UObjectInfo!"));
		}
	}

	UpdateInfoWidgetColor();
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
		PDPawn->GetTeamID();
	}
}

void ABallCore::DropPhysics(const FVector& DropLocation, const FVector& Impulse, const FVector& InCamDirection)
{
	Super::DropPhysics(DropLocation, Impulse, InCamDirection);

	if (HasAuthority())
	{
		SetObjectInfoTeamID(ETeamType::None);
	}

	if (IsValid(Sphere) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] DropPhysics failed. StaticMesh is invalid."));
		return;
	}

	Sphere->WakeAllRigidBodies();
	Sphere->AddImpulse(Impulse, NAME_None, true);
}

void ABallCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CachedInfoWidget)
	{
		return;
	}

	if (!CachedPlayer)
	{
		CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	if (CachedPlayer)
	{
		float Dist = GetDistanceTo(CachedPlayer);
		CachedInfoWidget->UpdateDistanceUI((int32)(Dist / 100.0f));
	}

	UpdateInfoWidgetColor();
}

void ABallCore::ResetBallForRound(const FVector& SpawnLocation)
{
	if (HasAuthority() == false)
	{
		return;
	}

	SetObjectInfoTeamID(ETeamType::None);
	Multicast_ApplyRoundReset(SpawnLocation);
}

void ABallCore::PlaceIntoGoal(USceneComponent* GoalAttachParent)
{
	if (!HasAuthority() || !IsValid(GoalAttachParent))
	{
		return;
	}

	ETeamType CarrierTeamID = ResolvePawnTeamID(CarrierPawn.Get());
	if (CarrierTeamID == ETeamType::None)
	{
		CarrierTeamID = ObjectInfoTeamID;
	}

	SetObjectInfoTeamID(CarrierTeamID);
	CarrierPawn = nullptr;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	Sphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Sphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	Sphere->SetSimulatePhysics(false);
	Sphere->SetEnableGravity(false);
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Sphere->SetCollisionProfileName(TEXT("NoCollision"));

	AttachToComponent(
		GoalAttachParent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	SceneRoot->SetRelativeLocation(FVector::ZeroVector);
	SceneRoot->SetRelativeRotation(FRotator::ZeroRotator);

	StaticMesh->SetRelativeLocation(FVector::ZeroVector);
	StaticMesh->SetRelativeRotation(FRotator::ZeroRotator);

	SetPlacedInGoal(true);
	SetActorEnableCollision(false);
	UpdateInfoWidgetColor();
}

void ABallCore::HandleCarrierChanged()
{
	if (IsValid(CarrierPawn.Get()))
	{
		if (APDGameStateBase* GS = GetWorld()->GetGameState<APDGameStateBase>())
		{
			if (APlayerState* PS = CarrierPawn->GetPlayerState())
			{
				if (APDPlayerState* MyPS = Cast<APDPlayerState>(PS))
				{
					GS->SetBallHolder(MyPS);

					if (HasAuthority())
					{
						SetObjectInfoTeamID(MyPS->GetTeamID());
					}

					if (APDGameModeBase* GM = GetWorld()->GetAuthGameMode<APDGameModeBase>())
					{
						GM->HandleBallPickedUp(MyPS, this);
					}
				}
			}
		}

		Sphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Sphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		Sphere->SetSimulatePhysics(false);
		Sphere->SetEnableGravity(false);
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Sphere->SetCollisionProfileName(TEXT("NoCollision"));
		
		SetActorEnableCollision(false);
		
		if (APDPawnBase* PD = Cast<APDPawnBase>(CarrierPawn))
		{
			if (USkeletalMeshComponent* CharacterMesh = PD->GetSkeletalMeshComponent())
			{
				AttachToComponent(
					CharacterMesh,
					FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					PD->BallSocketName
				);
			}
		}
	}
	else
	{
		if (HasAuthority() && bIsPlacedInGoal == false)
		{
			SetObjectInfoTeamID(ETeamType::None);
		}

		if (bIsPlacedInGoal)
		{
			UpdateInfoWidgetColor();
			return;
		}

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		SetActorEnableCollision(true);
		Sphere->SetCollisionProfileName(TEXT("PhysicsActor"));
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Sphere->SetEnableGravity(true);
		Sphere->SetSimulatePhysics(true);
		Sphere->WakeAllRigidBodies();
	}

	UpdateInfoWidgetColor();
}

UPrimitiveComponent* ABallCore::GetPhysicsComponent() const
{
	return Sphere;
}

void ABallCore::ApplyRoundResetLocal(const FVector& SpawnLocation)
{
	if (SpawnLocation == FVector::ZeroVector)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] ApplyRoundResetLocal failed. SpawnLocation is zero vector."));
		return;
	}

	if (IsValid(Sphere) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[BallCore] ApplyRoundResetLocal failed. StaticMesh is invalid."));
		return;
	}

	CarrierPawn = nullptr;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	Sphere->SetCollisionProfileName(TEXT("PhysicsActor"));
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Sphere->SetEnableGravity(true);
	Sphere->SetSimulatePhysics(true);

	Sphere->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Sphere->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	SetActorLocation(SpawnLocation);
	SetActorRotation(FRotator::ZeroRotator);

	Sphere->WakeAllRigidBodies();

	bIsCanInteract = true;
	bIsPlacedInGoal = false;
	ObjectInfoTeamID = ETeamType::None;

	UpdateInfoWidgetColor();

	UE_LOG(
		LogProjectD,
		Log,
		TEXT("[BallCore] ApplyRoundResetLocal completed. SpawnLocation=(%.2f, %.2f, %.2f)"),
		SpawnLocation.X,
		SpawnLocation.Y,
		SpawnLocation.Z
	);
}

void ABallCore::Multicast_ApplyRoundReset_Implementation(const FVector& SpawnLocation)
{
	ApplyRoundResetLocal(SpawnLocation);
}

void ABallCore::OnRep_ObjectInfoTeamID()
{
	UpdateInfoWidgetColor();
}

void ABallCore::UpdateInfoWidgetColor()
{
	if (IsValid(CachedInfoWidget) == false)
	{
		return;
	}

	const ETeamType ViewerTeamID = ResolveLocalViewerTeamID(this);
	if (LastAppliedViewerTeamID == ViewerTeamID && LastAppliedObjectInfoTeamID == ObjectInfoTeamID)
	{
		return;
	}

	CachedInfoWidget->SetInterfaceColor(UPDTeamColorFunctionLibrary::GetRelativeTeamColor(ViewerTeamID, ObjectInfoTeamID));
	LastAppliedViewerTeamID = ViewerTeamID;
	LastAppliedObjectInfoTeamID = ObjectInfoTeamID;
}

void ABallCore::SetObjectInfoTeamID(const ETeamType NewTeamID)
{
	if (ObjectInfoTeamID == NewTeamID)
	{
		return;
	}

	ObjectInfoTeamID = NewTeamID;
	UpdateInfoWidgetColor();
}
