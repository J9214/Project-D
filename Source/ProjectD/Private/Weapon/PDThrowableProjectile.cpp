#include "Weapon/PDThrowableProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"
#include "PDGameplayTags.h"

APDThrowableProjectile::APDThrowableProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	bReplicates = true;
	SetReplicateMovement(true);

	Collision = CreateDefaultSubobject<USphereComponent>("Collision");
	RootComponent = Collision;
	Collision->InitSphereRadius(10.f);
	
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);

	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->SetupAttachment(RootComponent);

	ProjectileMove = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMove");
	ProjectileMove->bRotationFollowsVelocity = true;
	ProjectileMove->bShouldBounce = true;
	ProjectileMove->bSweepCollision = true;
	ProjectileMove->bBounceAngleAffectsFriction = true;
	ProjectileMove->Bounciness = 0.2f;
	ProjectileMove->Friction = 1.0f;
	ProjectileMove->ProjectileGravityScale = 1.0f;
	ProjectileMove->SetUpdatedComponent(Collision);
}

void APDThrowableProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void APDThrowableProjectile::InitFromData(UDataAsset_Throwable* Data, const FVector& Start, const FVector& Velocity)
{
	if (!HasAuthority())
	{
		return;
	}
	
	ThrowableData = Data;
	SetActorLocation(Start);

	ProjectileMove->Velocity = Velocity;
	ProjectileMove->ProjectileGravityScale = Data ? Data->GravityScale : 1.0f;

	const float Fuse = Data ? Data->FuseTime : 3.0f;
	GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &APDThrowableProjectile::Explode, Fuse, false);
}

void APDThrowableProjectile::Explode()
{
	if (!HasAuthority() || !ThrowableData)
	{
		return;
	}
	
	ApplyGE();
	SendGameplayCueTag();
	
	Destroy();
}

void APDThrowableProjectile::ApplyGE()
{
	const FVector ExplodeLoc = GetActorLocation();

	if (ThrowableData->ExplosionGE)
	{
		AActor* OwnerActor = GetOwner();
		if (!OwnerActor)
		{
			return;
		}
		
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
		if (!SourceASC)
		{
			return;
		}
		
		FGameplayEffectContextHandle Context;
		if (SourceASC)
		{
			Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(this);
		}

		TArray<FOverlapResult> Overlaps;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ThrowableExplode), false, this);
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(OwnerActor);

		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);

		const float Radius = ThrowableData->ExplosionRadius ? ThrowableData->ExplosionRadius : 400.f;

		const bool bHit = GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			ExplodeLoc,
			FQuat::Identity,
			ObjParams,
			FCollisionShape::MakeSphere(Radius),
			Params
		);

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(ThrowableData->ExplosionGE, 1.0f, Context);

		if (bHit && SpecHandle.IsValid())
		{
			const float Damage = ThrowableData->ExplosionDamage ? ThrowableData->ExplosionDamage : 50.f;
			SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_Throwable_Grenade_Damage, Damage);
			
			for (const FOverlapResult& R : Overlaps)
			{
				APawn* Pawn = Cast<APawn>(R.GetActor());
				if (!Pawn)
				{
					continue;
				}
				
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
				if (!TargetASC)
				{
					continue;
				}
				
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

void APDThrowableProjectile::SendGameplayCueTag()
{
	const FVector ExplodeLoc = GetActorLocation();
	
	if (ThrowableData->ExplosionCueTag.IsValid())
	{
		AActor* OwnerActor = GetOwner();
		if (!OwnerActor)
		{
			return;
		}
		
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
		if (!SourceASC)
		{
			return;
		}

		FGameplayCueParameters Params;
		Params.Location = ExplodeLoc;
		Params.Instigator = OwnerActor;
		Params.EffectCauser = this;
		Params.SourceObject = ThrowableData;

		SourceASC->ExecuteGameplayCue(ThrowableData->ExplosionCueTag, Params);
	}
}

