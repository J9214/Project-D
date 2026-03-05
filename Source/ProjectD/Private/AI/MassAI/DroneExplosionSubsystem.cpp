#include "AI/MassAI/DroneExplosionSubsystem.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "PDGameplayTags.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "ProjectD/ProjectD.h"

void UDroneExplosionSubsystem::ApplyExplosionDamage(const FVector& Location, float Radius, float Damage, TSubclassOf<UGameplayEffect> DamageGE)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}

	// 서버에서만 데미지 적용
	if (World->GetNetMode() == NM_Client)
	{
		return;
	}

	if (DamageGE == nullptr)
	{
		UE_LOG(LogProjectD, Warning, TEXT("[DroneExplosionSub] DamageGE is nullptr"));
		return;
	}

	if (Radius <= 0.0f || Damage <= 0.0f)
	{
		return;
	}

	TArray<FOverlapResult> OverlappedResults;

	FCollisionObjectQueryParams ObjectQuery;
	ObjectQuery.AddObjectTypesToQuery(LineOfSightTraceChannel);

	const FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

	FCollisionQueryParams Params;
	Params.bTraceComplex = false;

	const bool bHit = World->OverlapMultiByObjectType(
		OverlappedResults,
		Location,
		FQuat::Identity,
		ObjectQuery,
		Sphere,
		Params
	);

	if (bHit == false)
	{
		return;
	}

	for (const FOverlapResult& Result : OverlappedResults)
	{
		AActor* TargetActor = Result.GetActor();
		if (IsValid(TargetActor) == false)
		{
			continue;
		}

		UAbilitySystemComponent* ASC = nullptr;

		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor);
		if (ASI != nullptr)
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
		else
		{
			ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
		}

		if (IsValid(ASC) == false)
		{
			continue;
		}

		if (bCheckLineOfSight == true)
		{
			if (HasLineOfSightToASC(ASC, Location) == false)
			{
				continue;
			}
		}

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			DamageGE,
			1.0f,
			Context
		);

		if (SpecHandle.IsValid() == false)
		{
			continue;
		}

		SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_AI_Drone_ExplodeDamage, Damage);

		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

bool UDroneExplosionSubsystem::HasLineOfSightToASC(UAbilitySystemComponent* ASC, const FVector& Origin) const
{
	if (IsValid(ASC) == false)
	{
		return false;
	}

	AActor* Avatar = ASC->GetAvatarActor();
	if (IsValid(Avatar) == false)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return false;
	}

	const FVector Start = Origin;
	const FVector End = Avatar->GetActorLocation();

	FCollisionQueryParams Params;
	Params.bTraceComplex = false;

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		LineOfSightTraceChannel,
		Params
	);

	if (bHit == false)
	{
		return true;
	}

	return (Hit.GetActor() == Avatar);
}
