#include "AI/MassAI/MassDamageBridgeSubsystem.h"
#include "AI/MassAI/CollisionProxyActor.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ProjectD/ProjectD.h"
#include "MassCommandBuffer.h"

bool UMassDamageBridgeSubsystem::TryApplyDamageFromProxyActor(AActor* HitActor, float Damage, const AActor* DamageCauser)
{
	if (IsValid(HitActor) == false)
	{
		return false;
	}

	if (Damage <= 0.0f)
	{
		return false;
	}

	ACollisionProxyActor* Proxy = Cast<ACollisionProxyActor>(HitActor);
	if (IsValid(Proxy) == false)
	{
		return false;
	}

	const FMassEntityHandle VictimEntity = Proxy->GetRepresentedEntity();
	if (VictimEntity.IsValid() == false)
	{
		// May Will Takes Some Subsystems
		// So return true & Dont use another if checks
		return true;
	}

	FPendingMassDamage Req;
	Req.Entity = VictimEntity;
	Req.Damage = Damage;

	if (IsValid(DamageCauser) == true)
	{
		const AActor* KillerActor = DamageCauser->GetOwner();
		if (IsValid(KillerActor) == false)
		{
			KillerActor = DamageCauser;
		}

		Req.Killer = const_cast<AActor*>(KillerActor);
	}

	PendingDamages.Add(Req);

	return true;
}

bool UMassDamageBridgeSubsystem::TryApplyDamageFromProxyHit(const FHitResult& Hit, float Damage, const AActor* DamageCauser)
{
	AActor* HitActor = Hit.GetActor();
	return TryApplyDamageFromProxyActor(HitActor, Damage, DamageCauser);
}

void UMassDamageBridgeSubsystem::MovePendingDamages(TArray<FPendingMassDamage>& Out)
{
	Out = MoveTemp(PendingDamages);
	PendingDamages.Reset();
}
