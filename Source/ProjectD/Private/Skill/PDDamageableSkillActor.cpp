#include "Skill/PDDamageableSkillActor.h"

#include "AbilitySystemComponent.h"
#include "PDGameplayTags.h"


APDDamageableSkillActor::APDDamageableSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

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
}

void APDDamageableSkillActor::OnEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec,
                                              FActiveGameplayEffectHandle Handle)
{
	FGameplayTag DamageTag = PDGameplayTags::Data_Weapon_Damage;

	float DamageAmount = Spec.GetSetByCallerMagnitude(DamageTag, false, 0.f);
	const FGameplayEffectContextHandle& Context = Spec.GetContext();
	const FHitResult* Hit = Context.GetHitResult();

	OnHitEvent(DamageAmount, *Hit);
}
