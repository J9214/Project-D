// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassPerceptionSubsystem.h"
#include "DataAssets/AI/MassAI/DataAsset_MassPerceptionSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "ProjectD/ProjectD.h"

bool UMassPerceptionSubsystem::IsServerWorld() const
{
	UWorld* World = GetWorld();

	if (IsValid(World) == false)
	{
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	return NetMode == NM_DedicatedServer;
}

const FMassPerceptionTuning* UMassPerceptionSubsystem::GetTuningOrNull() const
{
	if (HasValidSettings() == true)
	{
		return &Settings->Tuning;
	}

	return nullptr;
}

void UMassPerceptionSubsystem::SubmitAimTubeRequest(const FVector& Origin, const FVector& Dir, float Length)
{
	if(IsServerWorld() == false)
	{
		return;
	}

	if (Length <= 0.f)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassPerceptionSubsystem::SubmitAimTubeRequest - Length Is Weird!"));
		return;
	}

	if (Settings == nullptr)
	{
		LoadSettingsSync();
	}

	const FMassPerceptionTuning* Tuning = GetTuningOrNull();
	if (Tuning == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassPerceptionSubsystem::SubmitAimTubeRequest - Tuning Data Is Weird!"));
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassPerceptionSubsystem::SubmitAimTubeRequest - World Is Weird!"));
		return;
	}

	const int32 MaxReq = GetMaxRequests();
	if (ActiveAimTubes.Num() >= MaxReq)
	{
		TrimOneOldestRequest();
	}

	const float Now = World->GetTimeSeconds();

	FMassAimTubeRequest Req;
	Req.Origin = Origin;
	Req.Dir = Dir.GetSafeNormal();
	Req.WeaponLength = Length;
	Req.TubeRadius = Tuning->AimTubeRadius;
	Req.ExpireTime = Now + Tuning->AimTubeTTL;

	ActiveAimTubes.Add(MoveTemp(Req));
}

void UMassPerceptionSubsystem::CleanExpiredRequests(float NowSeconds)
{
	for (int32 i = ActiveAimTubes.Num() - 1; i >= 0; --i)
	{
		if (ActiveAimTubes[i].IsExpired(NowSeconds) == true)
		{
			ActiveAimTubes.RemoveAtSwap(i);
		}
	}
}

void UMassPerceptionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IsServerWorld() == false)
	{
		return;
	}

	LoadSettingsSync();
}

void UMassPerceptionSubsystem::Deinitialize()
{
	ActiveAimTubes.Reset();
	Settings = nullptr;
	Super::Deinitialize();
}

void UMassPerceptionSubsystem::LoadSettingsSync()
{
	if (IsValid(Settings) == true)
	{
		return;
	}

	if (SettingsAsset.IsNull() == true)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassPerceptionSubsystem::LoadSettingsSync - SettingsAsset is None. (Set it in DefaultGame.ini)"));
		return;
	}

	Settings = SettingsAsset.LoadSynchronous();

	if (IsValid(Settings) == true)
	{
		UE_LOG(LogProjectD, Warning,
			TEXT("UMassPerceptionSubsystem::LoadSettingsSync - Failed to load SettingsAsset: %s"), *SettingsAsset.ToSoftObjectPath().ToString());
		return;
	}

	UE_LOG(LogProjectD, Log, TEXT("UMassPerceptionSubsystem::LoadSettingsSync - Loaded MassPerception Settings: %s"), *Settings->GetName());
}

int32 UMassPerceptionSubsystem::GetMaxRequests() const
{
	const FMassPerceptionTuning* Tuning = GetTuningOrNull();
	if (Tuning == nullptr)
	{
		return -1;
	}

	const int32 SoftLimit = Tuning->MaxDesiredRequests;

	if (SoftLimit <= 0)
	{
		return TNumericLimits<int32>::Max();
	}

	return SoftLimit;
}

void UMassPerceptionSubsystem::TrimOneOldestRequest()
{
	if(ActiveAimTubes.Num() <= 0)
	{
		return;
	}

	int32 MinIdx = 0;
	float MinExpire = ActiveAimTubes[0].ExpireTime;

	for (int32 i = 1; i < ActiveAimTubes.Num(); ++i)
	{
		if (ActiveAimTubes[i].ExpireTime < MinExpire)
		{
			MinExpire = ActiveAimTubes[i].ExpireTime;
			MinIdx = i;
		}
	}

	ActiveAimTubes.RemoveAtSwap(MinIdx);
}