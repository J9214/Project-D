#include "Gimmick/ZipLine/PDZipLine.h"

#include "Pawn/PDPawnBase.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "CableComponent.h"
#include "MoverComponent.h"
#include "Mover/Public/MovementMode.h"

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
		Length += InteractBox->GetComponentScale().X;
		NewExtent.X = Length * 0.5f + 32.0f;
		InteractBox->SetBoxExtent(NewExtent);
	}
}

void APDZipLine::OnInteract_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	APDPawnBase* Pawn = Cast<APDPawnBase>(Interactor);
	if (!IsValid(Pawn))
	{
		return;
	}

	UMoverComponent* Mover = Interactor->GetComponentByClass<UMoverComponent>();
	if (!IsValid(Mover))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::OnInteract - Interactor has Invalid Mover!"));
		return;
	}

	if (!ZipLineLayeredMoveMap.Contains(Pawn))
	{
		FZipLineInfo TempInfo;
		TempInfo.bIsZiplining = false;
		TempInfo.LinearVelocity = MakeShared<FLayeredMove_LinearVelocity>();
		ZipLineLayeredMoveMap.Add(Pawn, TempInfo);
	}
	else if (!ZipLineLayeredMoveMap[Pawn].LinearVelocity.IsValid())
	{
		ZipLineLayeredMoveMap[Pawn].LinearVelocity = MakeShared<FLayeredMove_LinearVelocity>();
	}

	FZipLineInfo& ZipLineInfo = ZipLineLayeredMoveMap[Pawn];
	if (ZipLineInfo.bIsZiplining)
	{
		Execute_OnEndInteract(this, Interactor);
	}
	else
	{
		FVector RealDirection = CalcRealDirection(Pawn);
		FVector ZipVelocity = RealDirection * Speed;

		ZipLineInfo.LinearVelocity->Velocity = ZipVelocity;
		ZipLineInfo.LinearVelocity->MixMode = EMoveMixMode::OverrideVelocity;
		ZipLineInfo.LinearVelocity->DurationMs = Duration;
		ZipLineInfo.bIsZiplining = true;

		Mover->QueueLayeredMove(ZipLineInfo.LinearVelocity);
	}
}

void APDZipLine::OnEndInteract_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	APDPawnBase* Pawn = Cast<APDPawnBase>(Interactor);
	if (!IsValid(Pawn))
	{
		return;
	}

	UMoverComponent* Mover = Interactor->GetComponentByClass<UMoverComponent>();
	if (!IsValid(Mover))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::OnEndInteract - Interactor has Invalid Mover!"));
		return;
	}

	if (!ZipLineLayeredMoveMap.Contains(Pawn))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::OnEndInteract - %s hasn't use ZipLine!"), *Pawn->GetFName().ToString());
		return;
	}
	else if (!ZipLineLayeredMoveMap[Pawn].LinearVelocity.IsValid())
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::OnEndInteract - %s's ZipLine Data is not Valid!"), *Pawn->GetFName().ToString());
		return;
	}

	UE_LOG(LogProjectD, Warning, TEXT("APDZipLine::OnEndInteract - End ZipLine!"));
	FZipLineInfo& ZipLineInfo = ZipLineLayeredMoveMap[Pawn];
	ZipLineInfo.bIsZiplining = false;

	ZipLineInfo.LinearVelocity->EndMove(Mover, Mover->GetSimBlackboard_Mutable(), Duration);
}

FVector APDZipLine::CalcRealDirection(APawn* User)
{
	return Direction;
}

