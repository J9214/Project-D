#include "Explosion/PDExplosionActor.h"

#include "Pawn/PDPawnBase.h"
#include "Chaos/PDDestructibleObject.h"
#include "PDGameplayTags.h"

#include "Engine/AssetManager.h"
#include "Engine/OverlapResult.h"
#include "Components/SphereComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "GameplayEffect.h"

#include "ProjectD/ProjectD.h"

TSet<FName> APDExplosionActor::PDANameSet;

APDExplosionActor::APDExplosionActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	ExplosionRange = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionRange"));
	SetRootComponent(ExplosionRange);
}

void APDExplosionActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			DamageTimer,
			this,
			&APDExplosionActor::EvaluateDamage,
			0.1f,
			true
		);
	}
}

void APDExplosionActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (ExplosionRange->OnComponentBeginOverlap.IsBound())
	{
		ExplosionRange->OnComponentBeginOverlap.RemoveDynamic(this, &APDExplosionActor::HandleBeginOverlap);
	}

	if (ExplosionRange->OnComponentEndOverlap.IsBound())
	{
		ExplosionRange->OnComponentEndOverlap.RemoveDynamic(this, &APDExplosionActor::HandleEndOverlap);
	}

	for (TPair<UAbilitySystemComponent*, FTimerHandle> Pair : EffectTimers)
	{
		if (IsValid(Pair.Key))
		{
			TimerOff(Pair.Key);
		}
	}

	GetWorldTimerManager().ClearTimer(DamageTimer);

	if (IsValid(ContinuousParticle))
	{
		ContinuousParticle->Deactivate();
	}

	Super::EndPlay(EndPlayReason);
}

void APDExplosionActor::InitExplosion(FName InName, FName InType)
{
	if (!HasAuthority())
	{
		return;
	}

	PDAType = InType;
	PDAName = InName;
	if (!PDANameSet.Contains(PDAName))
	{
		LoadPDA(PDAType, PDAName);
	}
	else
	{
		OnPDAReady();
	}
}

void APDExplosionActor::OnPDAReady()
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!IsValid(AssetManager))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDExplosionActor::InitExplosion - Invalid Asset Manager!"));
		return;
	}

	UObject* Data = AssetManager->GetPrimaryAssetObject(FPrimaryAssetId(PDAType, PDAName));
	if (!IsValid(Data))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDExplosionActor::InitExplosion - No %s / %s Data!"), *PDAType.ToString(), *PDAName.ToString());
		return;
	}

	UPDExplosionDataAsset* ExplosionData = Cast<UPDExplosionDataAsset>(Data);
	if (!IsValid(ExplosionData))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDExplosionActor::InitExplosion - Data is not UPDExplosionDataAsset!"));
		return;
	}

	ExplosionRange->SetSphereRadius(ExplosionData->Range);
	Duration = ExplosionData->Duration;

	ExplosionDamageGE = ExplosionData->ExplosionDamageGE;
	ContinuousDamageGE = ExplosionData->ContinuousDamageGE;
	DestructDamageGE = ExplosionData->DestructDamageGE;

	ExplosionDamage = ExplosionData->ExplosionDamage;
	ContinuousDamage = ExplosionData->ContinuousDamage;
	DestructDamage = ExplosionData->DestructDamage;

	Type = ExplosionData->Type;

	SetLifeSpan(Duration);
	BindOverlapHandles();

	Explode();

	Multicast_ShowExplodeEffect(ExplosionData->StartEffectClass, ExplosionData->ContinuousEffectClass, ExplosionData->Range, Duration);
}

void APDExplosionActor::LoadPDA(FName InType, FName InName)
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!IsValid(AssetManager))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDExplosionActor::LoadPDA - Invalid Asset Manager!"));
		return;
	}

	FPrimaryAssetId AssetId = FPrimaryAssetId(InType, InName);
	TSharedPtr<FStreamableHandle> Handle = AssetManager->LoadPrimaryAsset(
		AssetId,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(
			this,
			&APDExplosionActor::OnPDAReady
		)
	);

	if (Handle.IsValid())
	{
		PDANameSet.Add(InName);
	}
}

void APDExplosionActor::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(ContinuousDamageGE))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDExplosionActor::HandleBeginOverlap - Invalid ContinuousDamageGE!"));
		return;
	}

	APDPawnBase* PDPawn = Cast<APDPawnBase>(OtherActor);
	if (!IsValid(PDPawn))
	{
		return;
	}

	UAbilitySystemComponent* ASC = PDPawn->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	DamageCandidates.Add(ASC);
}

void APDExplosionActor::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(ContinuousDamageGE))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDExplosionActor::HandleEndOverlap - Invalid ContinuousDamageGE!"));
		return;
	}
	
	APDPawnBase* PDPawn = Cast<APDPawnBase>(OtherActor);
	if (!IsValid(PDPawn))
	{
		return;
	}

	UAbilitySystemComponent* ASC = PDPawn->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	if (DamageCandidates.Contains(ASC))
	{
		DamageCandidates.Remove(ASC);
	}
	TimerOff(ASC);
}

void APDExplosionActor::CreateParticle(UFXSystemAsset* InParticle, bool bIsContinuous)
{
	if (IsValid(InParticle))
	{
		if (InParticle->IsA(UNiagaraSystem::StaticClass()))
		{
			UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				Cast<UNiagaraSystem>(InParticle),
				GetActorLocation()
			);

			if (bIsContinuous && IsValid(NiagaraComp))
			{
				NiagaraComp->SetVariableFloat(TEXT("SphereRadius"), ExplosionRange->GetScaledSphereRadius());
				NiagaraComp->SetVariableFloat(TEXT("Duration"), Duration);

				ContinuousParticle = NiagaraComp;
			}
		}
		else if (InParticle->IsA(UParticleSystem::StaticClass()))
		{
			UParticleSystemComponent* ParticleSystemComp = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				Cast<UParticleSystem>(InParticle),
				GetActorLocation()
			);

			if (bIsContinuous && IsValid(ParticleSystemComp))
			{
				//NiagaraComp->SetVariableFloat(TEXT("SphereRadius"), ExplosionRange->GetScaledSphereRadius());
				//NiagaraComp->SetVariableFloat(TEXT("Duration"), Duration);

				ContinuousParticle = ParticleSystemComp;
			}
		}
	}
}

void APDExplosionActor::BindOverlapHandles()
{
	if (!ExplosionRange->OnComponentBeginOverlap.IsBound())
	{
		ExplosionRange->OnComponentBeginOverlap.AddDynamic(this, &APDExplosionActor::HandleBeginOverlap);
	}

	if (!ExplosionRange->OnComponentEndOverlap.IsBound())
	{
		ExplosionRange->OnComponentEndOverlap.AddDynamic(this, &APDExplosionActor::HandleEndOverlap);
	}
}

bool APDExplosionActor::IsCanTakeDamage(UAbilitySystemComponent* ASC)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!IsValid(ASC))
	{
		return false;
	}

	AActor* Avatar = ASC->GetAvatarActor();
	if (!IsValid(Avatar))
	{
		return false;
	}

	const FVector Start = GetActorLocation();
	const FVector End = Avatar->GetActorLocation();

	TArray<FHitResult> HitResults;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = false;

	bool bHit = GetWorld()->LineTraceMultiByChannel(
		HitResults,
		Start,
		End,
		ECC_GameTraceChannel1,
		Params
	);

	if (!bHit)
	{
		return false;
	}
	
	HitResults.Sort([&](const FHitResult& A, const FHitResult& B)
		{
			return A.Distance < B.Distance;
		});

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!IsValid(HitActor))
		{
			continue;
		}

		if (HitActor == Avatar)
		{
			return true;
		}

		APDPawnBase* PDPawn = Cast<APDPawnBase>(HitActor);
		if (!IsValid(PDPawn))
		{
			return false;
		}
	}

	return false;
}

void APDExplosionActor::Explode()
{
	if (!HasAuthority())
	{
		return;
	}

	const float Radius = ExplosionRange->GetScaledSphereRadius();

	TArray<FOverlapResult> OverlappedResult;

	FCollisionObjectQueryParams ObjectQuery;
	ObjectQuery.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQuery.AddObjectTypesToQuery(ECC_Destructible);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlappedResult,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQuery,
		Sphere
	);

	if (!bHit)
	{
		return;
	}

	for (const FOverlapResult& Result : OverlappedResult)
	{
		AActor* OverlappedActor = Result.GetActor();
		if (!IsValid(OverlappedActor))
		{
			continue;
		}

		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OverlappedActor);
		if (ASI)
		{
			UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
			if (!IsValid(ASC))
			{
				continue;
			}

			DamageCandidates.Add(ASC);
			ApplyEffect(ASC, true);
		}
	}
}

void APDExplosionActor::ApplyEffect(UAbilitySystemComponent* ASC, bool bIsExplode)
{
	if (!HasAuthority())
	{
		return;
	}

	const float Damage = (bIsExplode ? ExplosionDamage : ContinuousDamage);
	TSubclassOf<UGameplayEffect> GE = (bIsExplode && IsValid(ExplosionDamageGE) ? ExplosionDamageGE : ContinuousDamageGE);
	FGameplayTag Tag = (bIsExplode ? PDGameplayTags::Data_ThrowableObject_ExplodeDamage : PDGameplayTags::Data_ThrowableObject_ContinuousDamage);

	if (IsValid(GE))
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			GE,
			1.0f,
			Context
		);

		if (!SpecHandle.IsValid())
		{
			return;
		}

		SpecHandle.Data->SetSetByCallerMagnitude(Tag, Damage);

		FActiveGameplayEffectHandle ActiveEffect = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		if (ActiveEffectHandles.Contains(ASC))
		{
			ActiveEffectHandles[ASC] = ActiveEffect;
		}
		else
		{
			ActiveEffectHandles.Add(ASC, ActiveEffect);
		}
	}

	if (bIsExplode)
	{
		APDDestructibleObject* DestructableObject = Cast<APDDestructibleObject>(ASC->GetAvatarActor());
		if (IsValid(DestructDamageGE) && IsValid(DestructableObject))
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				DestructDamageGE,
				1.0f,
				Context
			);

			if (!SpecHandle.IsValid())
			{
				return;
			}

			SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_ThrowableObject_DestructDamage, DestructDamage);

			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	TimerOn(ASC);
}

void APDExplosionActor::TimerOn(UAbilitySystemComponent* ASC)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!EffectTimers.Contains(ASC))
	{
		FTimerHandle Timer;
		EffectTimers.Add(ASC, Timer);
	}

	GetWorldTimerManager().ClearTimer(EffectTimers[ASC]);
	GetWorldTimerManager().SetTimer(
		EffectTimers[ASC],
		[&, ASC]
		{
			this->ApplyEffect(ASC);
		},
		0.3f,
		false
	);
}

void APDExplosionActor::TimerOff(UAbilitySystemComponent* ASC)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!EffectTimers.Contains(ASC))
	{
		return;
	}
	
	switch (Type)
	{
		case EExplosionAreaDamageType::Poison:
		{
			if (ActiveEffectHandles.Contains(ASC))
			{
				ASC->RemoveActiveGameplayEffect(ActiveEffectHandles[ASC]);
			}
		}
		break;
	}

	GetWorldTimerManager().ClearTimer(EffectTimers[ASC]);
}

void APDExplosionActor::EvaluateDamage()
{
	for (UAbilitySystemComponent* ASC : DamageCandidates)
	{
		const bool bCanDamage = IsCanTakeDamage(ASC);

		if (bCanDamage)
		{
			ApplyEffect(ASC);
		}
		else
		{
			TimerOff(ASC);
		}
	}
}

void APDExplosionActor::Multicast_ShowExplodeEffect_Implementation(UFXSystemAsset* InStartParticle, UFXSystemAsset* InContinuousParticle, const float& InRange, const float& InDuration)
{
	ExplosionRange->SetSphereRadius(InRange);
	Duration = InDuration;

	CreateParticle(InStartParticle, false);
	CreateParticle(InContinuousParticle, true);
}

