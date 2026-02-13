#include "AI/MassAI/MassDamageBridgeSubsystem.h"
#include "AI/MassAI/CollisionProxyActor.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ProjectD/ProjectD.h"
#include "MassCommandBuffer.h"

bool UMassDamageBridgeSubsystem::TryApplyDamageFromProxyActor(AActor* HitActor, float Damage)
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

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassDamageBridgeSubsystem::TryApplyDamageFromProxyActor - World Is Weird!"));
		return true;
	}

	UMassEntitySubsystem* MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (IsValid(MassEntitySubsystem) == false)
	{
		UE_LOG(LogProjectD, Warning, TEXT("UMassDamageBridgeSubsystem::TryApplyDamageFromProxyActor - UMassEntitySubsystem Is Not Valid!"));
		return true;
	}

	return true;
}

bool UMassDamageBridgeSubsystem::TryApplyDamageFromProxyHit(const FHitResult& Hit, float Damage)
{
	AActor* HitActor = Hit.GetActor();
	return TryApplyDamageFromProxyActor(HitActor, Damage);
}
