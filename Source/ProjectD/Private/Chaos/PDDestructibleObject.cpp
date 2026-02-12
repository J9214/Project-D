#include "Chaos/PDDestructibleObject.h"

#include "ProjectD/ProjectD.h"
#include "Chaos/AttributeSet/PDDestructibleAttributeSet.h"

#include "Net/UnrealNetwork.h"

#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

#include "AbilitySystemComponent.h"

namespace PDGameplayTags
{
	/** SetByCaller Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Data_Weapon_DestructDamage, "Data.Weapon.DestructDamage");
}

APDDestructibleObject::APDDestructibleObject()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UPDDestructibleAttributeSet>(TEXT("AttributeSet"));

	bIsDestroyed = false;
}

UAbilitySystemComponent* APDDestructibleObject::GetAbilitySystemComponent() const
{
	return ASC;
}

void APDDestructibleObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitAbilityActorInfo();

	if (HasAuthority())
	{
		InitAttributeSet();
	}
}

void APDDestructibleObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(APDDestructibleObject, bIsDestroyed, COND_None, REPNOTIFY_Always);
}

void APDDestructibleObject::InitAbilityActorInfo()
{
	ASC->InitAbilityActorInfo(this, this);
}

void APDDestructibleObject::InitAttributeSet()
{
	if (!IsValid(ASC))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDDestructibleObject::InitAttributeSet - ASC is not valid"));
		return;
	}

	ASC->AddAttributeSetSubobject<UPDDestructibleAttributeSet>(AttributeSet);

	BindAttributeSetDelegates();
}

void APDDestructibleObject::BindAttributeSetDelegates()
{
	if (!IsValid(ASC))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDDestructibleObject::InitAttributeSet - ASC is not valid"));
		return;
	}

	AttributeSet->OutOfDurabilityChanged.AddDynamic(this, &ThisClass::ChangeToGeometryCollection);
}

void APDDestructibleObject::OnRep_Destroyed()
{
	if (!bIsDestroyed) return;

	HandleDestroyed();
}

void APDDestructibleObject::HandleDestroyed()
{
	if (!IsValid(SpawnGCActor))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDDestructibleObject::HandleDestroyed - SpawnGCActor is not valid"));
		return;
	}

	StaticMesh->SetVisibility(false);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (HasAuthority())
	{
		FTransform Transform = GetActorTransform();
		AGeometryCollectionActor* GCA = GetWorld()->SpawnActor<AGeometryCollectionActor>(SpawnGCActor, Transform);

		if (IsValid(GCA))
		{
			GCA->GetGeometryCollectionComponent()->SetSimulatePhysics(true);
		}
	}
}

void APDDestructibleObject::ChangeToGeometryCollection(AActor* InInstigator, const FVector& HitLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsDestroyed = true;
	OnRep_Destroyed();
}

