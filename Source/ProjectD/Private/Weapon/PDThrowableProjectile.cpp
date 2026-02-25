#include "Weapon/PDThrowableProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"
#include "Weapon/PDThrowableFireArea.h"
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
	
	Collision->OnComponentHit.AddDynamic(this, &APDThrowableProjectile::OnProjectileHit);
}

void APDThrowableProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (!HasAuthority() || !ThrowableData)
	{
		return;
	}
	
	if (bExploded || !bExplosionOnImpact)
	{
		return;
	}

	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	bExploded = true;
	CachedExplosionLocation = GetActorLocation();
	
	GetWorldTimerManager().ClearTimer(FuseTimerHandle);
	
	Explode();
}

void APDThrowableProjectile::InitFromData(UDataAsset_Throwable* Data, const FVector& Start, const FVector& Velocity)
{
	if (!HasAuthority() || !Data)
	{
		return;
	}
	
	ThrowableData = Data;
	SetActorLocation(Start);

	ProjectileMove->Velocity = Velocity;
	ProjectileMove->ProjectileGravityScale = Data->GravityScale;
	
	bExplosionOnImpact = Data->bExplosionOnImpact;

	const float Fuse = Data->FuseTime;
	GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &APDThrowableProjectile::Explode, Fuse, false);
}

void APDThrowableProjectile::Explode()
{
	if (!HasAuthority() || !ThrowableData)
	{
		return;
	}
	
	if (!bExploded)
	{
		bExploded = true;
		CachedExplosionLocation = GetActorLocation();
	}
	
	switch (ThrowableData->EffectType)
	{
	case EPDThrowableEffectType::Fragment:
		ApplyExplosionGE();
		SendExplosionCueTag();
		break;

	case EPDThrowableEffectType::Flame:
		SpawnFireArea();
		SendExplosionCueTag();
		break;

	default:
		break;
	}
	
	Destroy();
}

void APDThrowableProjectile::ApplyExplosionGE()
{
	if (!ThrowableData || !ThrowableData->ExplosionGE)
	{
		return;
	}

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

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ThrowableExplode), false, this);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(OwnerActor);

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	const float Radius = ThrowableData->ExplosionRadius ? ThrowableData->ExplosionRadius : 400.f;

	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		CachedExplosionLocation,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(ThrowableData->ExplosionGE, 1.0f, Context);

	if (bHit && SpecHandle.IsValid())
	{
		const float Damage = ThrowableData->ExplosionDamage ? ThrowableData->ExplosionDamage : 50.f;
		SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_Throwable_Fragment_Damage, Damage);

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

void APDThrowableProjectile::SpawnFireArea()
{
	if (!HasAuthority() || !ThrowableData || !ThrowableData->FireAreaClass)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = OwnerActor;
	Params.Instigator = Cast<APawn>(OwnerActor);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APDThrowableFireArea* FireArea = World->SpawnActorDeferred<APDThrowableFireArea>(
		ThrowableData->FireAreaClass,
		FTransform(FRotator::ZeroRotator, CachedExplosionLocation),
		OwnerActor,
		Cast<APawn>(OwnerActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (FireArea)
	{
		FireArea->InitFromData(OwnerActor, ThrowableData);
		FireArea->FinishSpawning(FTransform(FRotator::ZeroRotator, CachedExplosionLocation)); 
	}
}

void APDThrowableProjectile::SendExplosionCueTag()
{
	if (!ThrowableData || !ThrowableData->ExplosionCueTag.IsValid())
	{
		return;
	}

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
	Params.Location = GetActorLocation();;
	Params.Instigator = OwnerActor;
	Params.EffectCauser = this;
	Params.SourceObject = ThrowableData;

	SourceASC->ExecuteGameplayCue(ThrowableData->ExplosionCueTag, Params);
}
