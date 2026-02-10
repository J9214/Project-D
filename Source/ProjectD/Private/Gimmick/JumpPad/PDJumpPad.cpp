#include "Gimmick/JumpPad/PDJumpPad.h"

#include "Gimmick/Data/PDJumpPadDataAsset.h"

#include "Engine/AssetManager.h"
#include "Components/SphereComponent.h"

#include "MoverComponent.h"

#include "ProjectD/ProjectD.h"

TSet<FName> APDJumpPad::PDANameSet;

APDJumpPad::APDJumpPad()
{
	PDAType = "JumpPad";

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SphereComp->SetupAttachment(RootComponent);

	Shape = SphereComp;
}

void APDJumpPad::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (!PDANameSet.Contains(PDAName))
	{
		LoadPDA();
	}
}

void APDJumpPad::OnInteract_Implementation(AActor* Interactor)
{
	UMoverComponent* Mover = Interactor->GetComponentByClass<UMoverComponent>();
	if (!IsValid(Mover))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDJumpPad::OnInteract - Interactor has Invalid Mover!"));
		return;
	}

	if (!SPJump.IsValid())
	{
		UAssetManager* AssetManager = UAssetManager::GetIfValid();
		if (!IsValid(AssetManager))
		{
			UE_LOG(LogProjectD, Warning, TEXT("APDJumpPad::OnInteract - Invalid Asset Manager!"));
			return;
		}

		UObject* Data = AssetManager->GetPrimaryAssetObject(FPrimaryAssetId(PDAType, PDAName));
		if (!IsValid(Data))
		{
			UE_LOG(LogProjectD, Warning, TEXT("APDJumpPad::OnInteract - No %s / %s Data!"), *PDAType.ToString(), *PDAName.ToString());
			return;
		}

		JumpData = Cast<UPDJumpPadDataAsset>(Data);
		SPJump = MakeShared<FLayeredMove_JumpTo>();
	}

	if (!IsValid(JumpData))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDJumpPad::OnInteract - Data is not PDJumpPadDataAsset!"));
		return;
	}

	SPJump->bUseActorRotation = JumpData->JumpInfo.bUseActorRotation;

	SPJump->JumpHeight = JumpData->JumpInfo.JumpHeight;
	FVector Velocity = Mover->GetVelocity();
	Velocity.Z = 0.0f;
	SPJump->JumpDistance = (SPJump->bUseActorRotation ? JumpData->JumpInfo.JumpDistance + Velocity.Length() : JumpData->JumpInfo.JumpDistance);
	SPJump->JumpRotation = (SPJump->bUseActorRotation ? JumpData->JumpInfo.JumpRotation : GetActorRotation());

	Mover->QueueLayeredMove(SPJump);
}

void APDJumpPad::OnEndInteract_Implementation(AActor* Interactor)
{
}

void APDJumpPad::LoadPDA()
{
	UAssetManager* AssetManager = UAssetManager::GetIfValid();
	if (!IsValid(AssetManager))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDJumpPad::LoadPDA - Invalid Asset Manager!"));
		return;
	}
	
	TSharedPtr<FStreamableHandle> Handle = AssetManager->LoadPrimaryAsset(FPrimaryAssetId(PDAType, PDAName), TArray<FName>());
	if (Handle.IsValid())
	{
		PDANameSet.Add(PDAName);
	}
}
