#include "PlayerState/PDPlayerState.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "Net/UnrealNetwork.h"

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

void APDPlayerState::OnRep_TeamId()
{
	UE_LOG(LogTemp, Warning, TEXT("팀 번호 갱신됨: %d"), TeamId);
}

void APDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APDPlayerState, TeamId);
}

