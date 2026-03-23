#include "Gimmick/PDGimmickBase.h"

#include "Net/UnrealNetwork.h"

APDGimmickBase::APDGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SceneRoot->SetupAttachment(RootComponent);
}

void APDGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APDGimmickBase, bIsCanInteract);
}

