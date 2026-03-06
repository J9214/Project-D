#include "Skill/PDDamageableSkillActor.h"

#include "AbilitySystemComponent.h"
#include "PDGameplayTags.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"


APDDamageableSkillActor::APDDamageableSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	ShieldMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMeshComponent"));
	SetRootComponent(ShieldMeshComponent);
	ShieldMeshComponent->SetIsReplicated(true);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void APDDamageableSkillActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(
			this, &ThisClass::OnEffectApplied
		);
	}
}

void APDDamageableSkillActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	//OnRep_ShieldVisuals();
}

void APDDamageableSkillActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APDDamageableSkillActor, MaxHealth);
	DOREPLIFETIME(APDDamageableSkillActor, CurrentHealth);
	DOREPLIFETIME(APDDamageableSkillActor, ShieldStaticMesh);
	DOREPLIFETIME(APDDamageableSkillActor, ShieldBaseMaterial);
	DOREPLIFETIME(APDDamageableSkillActor, OwnerTeamID);
	DOREPLIFETIME(APDDamageableSkillActor, DamageableType);
}

void APDDamageableSkillActor::InitializeShieldSettings(float InMaxHealth, UStaticMesh* InStaticMesh, UMaterialInterface* InBaseMaterial, ETeamType InOwnerTeamID, EPDShieldType InDamageableType)
{
	MaxHealth = FMath::Max(1.f, InMaxHealth);
	CurrentHealth = MaxHealth;
	ShieldStaticMesh = InStaticMesh;
	ShieldBaseMaterial = InBaseMaterial;
	OwnerTeamID = InOwnerTeamID;
	DamageableType = InDamageableType;

	OnRep_ShieldVisuals();
}

void APDDamageableSkillActor::OnRep_ShieldVisuals()
{
	if (!ShieldMeshComponent)
	{
		return;
	}

	if (ShieldStaticMesh)
	{
		ShieldMeshComponent->SetStaticMesh(ShieldStaticMesh);
	}
	
	if (ShieldBaseMaterial)
	{
		ShieldMeshComponent->SetMaterial(0, ShieldBaseMaterial);
	}
	
	SetShieldMesh();
	
}

void APDDamageableSkillActor::OnEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec,
                                              FActiveGameplayEffectHandle Handle)
{
	FGameplayTag DamageTag = PDGameplayTags::Data_Weapon_Damage;

	float DamageAmount = Spec.GetSetByCallerMagnitude(DamageTag, false, 0.f);
	const FGameplayEffectContextHandle& Context = Spec.GetContext();
	const FHitResult* Hit = Context.GetHitResult();

	const FHitResult EmptyHitResult;

	if (!HasAuthority() || DamageAmount <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
	
	OnHitEvent(DamageAmount, Hit ? *Hit : EmptyHitResult);
}
