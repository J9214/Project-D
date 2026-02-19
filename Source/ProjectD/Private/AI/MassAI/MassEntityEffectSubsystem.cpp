#include "AI/MassAI/MassEntityEffectSubsystem.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

bool UMassEntityEffectSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	if (IsValid(World) == false)
	{
		return false;
	}

	if (World->IsGameWorld() == false)
	{
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_DedicatedServer)
	{
		return false;
	}

	return true;
}

void UMassEntityEffectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedCues.Reset();
}

void UMassEntityEffectSubsystem::Deinitialize()
{
	CachedCues.Reset();

	Super::Deinitialize();
}

const FMassEntityEffectCue* UMassEntityEffectSubsystem::FindCueDef(const EMassEntityCueId CueId) const
{
	for (const FMassEntityEffectCue& Def : Cues)
	{
		if (Def.CueId == CueId)
		{
			return &Def;
		}
	}

	return nullptr;
}

void UMassEntityEffectSubsystem::PlayCueAtLocation(const EMassEntityCueId CueId, const FVector& Location)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	const ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_DedicatedServer)
	{
		return;
	}

	FCachedCue* Cached = CachedCues.Find(CueId);
	if (Cached == nullptr)
	{
		const FMassEntityEffectCue* Def = FindCueDef(CueId);
		if (Def == nullptr)
		{
			return;
		}

		FCachedCue NewCached;
		NewCached.Scale = Def->EffectScale;
		NewCached.bPlaySound = (Def->bPlaySound == true);
		NewCached.bPlayNiagara = (Def->bPlayNiagara == true);

		if (Def->NiagaraEffectAsset.IsNull() == false)
		{
			NewCached.Niagara = Def->NiagaraEffectAsset.LoadSynchronous();
		}

		if (Def->SoundAsset.IsNull() == false)
		{
			NewCached.Sound = Def->SoundAsset.LoadSynchronous();
		}

		CachedCues.Add(CueId, NewCached);
		Cached = CachedCues.Find(CueId);
	}

	if (Cached == nullptr)
	{
		return;
	}

	if ((Cached->bPlayNiagara == true) && (IsValid(Cached->Niagara) == true))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			Cached->Niagara,
			Location,
			FRotator::ZeroRotator,
			Cached->Scale,
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
	}

	if ((Cached->bPlaySound == true) && (IsValid(Cached->Sound) == true))
	{
		UGameplayStatics::PlaySoundAtLocation(World, Cached->Sound, Location);
	}
}
