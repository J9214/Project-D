#include "Gimmick/ZipLine/PDZipLine.h"

#include "Pawn/PDPawnBase.h"
#include "Components/Input/MovementBridgeComponent.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "CableComponent.h"
#include "MoverComponent.h"

#include "ProjectD/ProjectD.h"

APDZipLine::APDZipLine()
{
	OtherStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OtherStaticMesh"));
	OtherStaticMesh->SetupAttachment(RootComponent);

	CableAttachA = CreateDefaultSubobject<USceneComponent>(TEXT("CableAttachA"));
	CableAttachA->SetupAttachment(StaticMesh);

	CableAttachB = CreateDefaultSubobject<USceneComponent>(TEXT("CableAttachB"));
	CableAttachB->SetupAttachment(OtherStaticMesh);

	Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
	Cable->SetupAttachment(CableAttachA);

	InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
	InteractBox->SetupAttachment(OtherStaticMesh);
	InteractBox->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	Speed = 1500.0f;
}

void APDZipLine::BeginPlay()
{
	Super::BeginPlay();

	FVector AttachALocation = CableAttachA->GetComponentLocation();
	FVector AttachBLocation = CableAttachB->GetComponentLocation();
	float Length = (AttachALocation - AttachBLocation).Size();

	Direction = (AttachBLocation - AttachALocation).GetSafeNormal();
	Duration = Length / Speed * 1000.0f;

	if (IsValid(InteractBox) && IsValid(CableAttachA) && IsValid(CableAttachB))
	{
		FVector Location = (AttachALocation + AttachBLocation) / 2.0f;
		FRotator NewRotator = UKismetMathLibrary::FindLookAtRotation(AttachBLocation, AttachALocation);

		InteractBox->SetWorldLocation(Location);
		InteractBox->SetWorldRotation(NewRotator);

		FVector NewExtent = InteractBox->GetUnscaledBoxExtent();
		Length /= InteractBox->GetComponentScale().X;
		NewExtent.X = Length * 0.5f + 32.0f;
		InteractBox->SetBoxExtent(NewExtent);
	}
}

void APDZipLine::EndPlay(EEndPlayReason::Type EndReason)
{
	for (TPair<APDPawnBase*, FZipLineInfo> Pair : ZipLineLayeredMoveMap)
	{
		GetWorldTimerManager().ClearTimer(Pair.Value.Timer);
	}

	Super::EndPlay(EndReason);
}

void APDZipLine::OnInteract_Implementation(AActor* Interactor)
{
	APDPawnBase* PDPawn = Cast<APDPawnBase>(Interactor);
	if (!IsValid(PDPawn))
	{
		return;
	}

	UMovementBridgeComponent* BridgeComp = PDPawn->GetMovementBridgeComponent();
	if (!IsValid(BridgeComp))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDJumpPad::OnInteract - UMovementBridgeComponent has invalid MovementBridgeComponent!"));
		return;
	}

	if (!ZipLineLayeredMoveMap.Contains(PDPawn))
	{
		FZipLineInfo TempInfo;

		TempInfo.BridgeComp = BridgeComp;
		TempInfo.bIsZiplining = false;
		TempInfo.LastVelocity = FVector::ZeroVector;

		ZipLineLayeredMoveMap.Add(PDPawn, TempInfo);
	}
	else if (!IsValid(ZipLineLayeredMoveMap[PDPawn].BridgeComp))
	{
		ZipLineLayeredMoveMap[PDPawn].BridgeComp = BridgeComp;
	}

	FZipLineInfo& ZipLineInfo = ZipLineLayeredMoveMap[PDPawn];
	if (ZipLineInfo.bIsZiplining)
	{
		Execute_OnEndInteract(this, Interactor);
	}
	else
	{
		float Ratio = MoveUser(PDPawn);

		FVector RealDirection = CalcRealDirection(PDPawn);
		FVector ZipVelocity = RealDirection * Speed;
		float Sign = FVector::DotProduct(RealDirection, Direction);

		Ratio = (Sign > 0.f) ? Ratio : (1.f - Ratio);
		float RealDuration = Duration * Ratio;

		FMoveRequest Req;

		Req.Type = EMoveRequestType::LinearVelocityDash;
		Req.LaunchVelocity = ZipVelocity;
		Req.DurationMs = RealDuration;

		ZipLineInfo.bIsZiplining = true;
		ZipLineInfo.LastVelocity = ZipVelocity;
		ZipLineInfo.BridgeComp->EnqueueMoveRequest(Req);

		TWeakObjectPtr<APDZipLine> WeakThis = TWeakObjectPtr<APDZipLine>(this);
		TWeakObjectPtr<AActor> WeakInteractor = TWeakObjectPtr<AActor>(Interactor);

		GetWorldTimerManager().ClearTimer(ZipLineInfo.Timer);
		GetWorldTimerManager().SetTimer(
			ZipLineInfo.Timer,
			[WeakThis, WeakInteractor] {
				if (WeakThis.IsValid() && WeakInteractor.IsValid())
				{
					WeakThis->Execute_OnEndInteract(WeakThis.Get(), WeakInteractor.Get());
				}
			},
			RealDuration / 1000.0f,
			false
		);
	}
}

void APDZipLine::OnEndInteract_Implementation(AActor* Interactor)
{
	APDPawnBase* PDPawn = Cast<APDPawnBase>(Interactor);
	if (!IsValid(PDPawn))
	{
		return;
	}

	if (!ZipLineLayeredMoveMap.Contains(PDPawn))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::OnEndInteract - %s hasn't use ZipLine!"), *PDPawn->GetFName().ToString());
		return;
	}
	else if (!IsValid(ZipLineLayeredMoveMap[PDPawn].BridgeComp))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::OnEndInteract - %s's ZipLine Data is not Valid!"), *PDPawn->GetFName().ToString());
		return;
	}

	UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::OnEndInteract - End ZipLine!"));
	FZipLineInfo& ZipLineInfo = ZipLineLayeredMoveMap[PDPawn];

	ZipLineInfo.bIsZiplining = false;

	GetWorldTimerManager().ClearTimer(ZipLineInfo.Timer);
}

FVector APDZipLine::CalcRealDirection(APawn* User)
{
	if (!IsValid(User))
	{
		return Direction;
	}
	
	FVector UserForward = User->GetActorForwardVector().GetSafeNormal();

	FVector UserLocation = User->GetActorLocation();

	FVector UserToA = (CableAttachA->GetComponentLocation() - UserLocation).GetSafeNormal();
	FVector UserToB = (CableAttachB->GetComponentLocation() - UserLocation).GetSafeNormal();

	float DotA = FVector::DotProduct(UserForward, UserToA);
	float DotB = FVector::DotProduct(UserForward, UserToB);

	if (DotA > 0.f && DotB > 0.f)
	{
		return (DotB > DotA) ? Direction : -Direction;
	}
	else
	{
		if (DotB > 0.f)
		{
			return Direction;
		}
		else if (DotA > 0.f)
		{
			return -Direction;
		}
	}

	return (DotB > DotA) ? Direction : -Direction;
}

float APDZipLine::MoveUser(APawn* User)
{
	UMoverComponent* Mover = User->GetComponentByClass<UMoverComponent>();
	if (!IsValid(Mover))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::MoveUser - Interactor has Invalid Mover!"));
		return 1.0f;
	}

	USceneComponent* UpdatedComp = Mover->GetUpdatedComponent();
	if (!IsValid(UpdatedComp))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::MoveUser - Mover has Invalid Updated Component!"));
		return 1.0f;
	}

	FVector AttachALocation = CableAttachA->GetComponentLocation();
	FVector AttachBLocation = CableAttachB->GetComponentLocation();

	FVector NewLocation = FMath::ClosestPointOnSegment(UpdatedComp->GetComponentLocation(), AttachALocation, AttachBLocation);
	UCapsuleComponent* PawnCapsule = User->GetComponentByClass<UCapsuleComponent>();
	if (IsValid(PawnCapsule))
	{
		float Height = PawnCapsule->GetScaledCapsuleHalfHeight();
		NewLocation.Z -= (Height - 100.0f);
	}

	float TotalDistance = FVector::Distance(AttachALocation, AttachBLocation);
	float RemainDistance = FVector::Distance(NewLocation, AttachBLocation);

	return (RemainDistance / TotalDistance);
}

