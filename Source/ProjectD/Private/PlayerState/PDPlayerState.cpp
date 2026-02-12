#include "PlayerState/PDPlayerState.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "AttributeSet/PDAttributeSetBase.h"

APDPlayerState::APDPlayerState()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AbilitySystemComponent = CreateDefaultSubobject<UPDAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSetBase = CreateDefaultSubobject<UPDAttributeSetBase>(TEXT("AttributeSetBase"));
	
}

UAbilitySystemComponent* APDPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APDPlayerState::InitAbilityActorInfo(AActor* AvatarActor)
{
	if (!AbilitySystemComponent || !AvatarActor)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, AvatarActor);
}

void APDPlayerState::SetDeadState()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!AbilitySystemComponent) 
	{
		return;
	}

	static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));

	if (AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		return; 
	}

	if (!GE_DeathClass)
	{
		return;
	}

	AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(GE_DeathClass, AbilitySystemComponent);
	
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	const UGameplayEffect* DeathGE = GE_DeathClass->GetDefaultObject<UGameplayEffect>();
	if (DeathGE)
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(DeathGE, 1.0f, Context);
	}
}

void APDPlayerState::SetReviveState()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!AbilitySystemComponent)
	{
		return;
	}

	static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	if (!AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		return; 
	}

	if (GE_DeathClass)
	{
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(GE_DeathClass, AbilitySystemComponent);
	}

	if (!GE_ReviveClass)
	{
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	const UGameplayEffect* ReviveGE = GE_ReviveClass->GetDefaultObject<UGameplayEffect>();
	if (ReviveGE)
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(ReviveGE, 1.0f, Context);
	}
}
