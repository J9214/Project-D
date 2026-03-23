#include "PDPlacedSkillActorBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

APDPlacedSkillActorBase::APDPlacedSkillActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APDPlacedSkillActorBase::BeginPlay()
{
	Super::BeginPlay();
}

void APDPlacedSkillActorBase::InitializePlacedActor(float InLifeTime, ETeamType InOwnerTeamID)
{
	LifeTime = InLifeTime;
	OwnerTeamID = InOwnerTeamID;

	if (HasAuthority() && bAutoDestroyByLifeTime && LifeTime > 0.f)
	{
		GetWorldTimerManager().ClearTimer(LifeTimerHandle);
		GetWorldTimerManager().SetTimer(
			LifeTimerHandle,
			this,
			&ThisClass::HandleLifeExpired,
			LifeTime,
			false
		);
	}
}

void APDPlacedSkillActorBase::SetOwnerTeamID(ETeamType InOwnerTeamID)
{
	OwnerTeamID = InOwnerTeamID;
}

void APDPlacedSkillActorBase::RequestDestroy_Implementation(bool bDestroy)
{
	if (bDestroy && HasAuthority())
	{
		Destroy();
	}
}

void APDPlacedSkillActorBase::HandleLifeExpired()
{
	if (HasAuthority())
	{
		RequestDestroy(true);
	}
}

void APDPlacedSkillActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APDPlacedSkillActorBase, LifeTime);
	DOREPLIFETIME(APDPlacedSkillActorBase, bAutoDestroyByLifeTime);
	DOREPLIFETIME(APDPlacedSkillActorBase, OwnerTeamID);
}