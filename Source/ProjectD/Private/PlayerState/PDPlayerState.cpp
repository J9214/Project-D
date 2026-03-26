#include "PlayerState/PDPlayerState.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "Net/UnrealNetwork.h"
#include "Components/Inventory/PDInventoryComponent.h"

APDPlayerState::APDPlayerState()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AbilitySystemComponent = CreateDefaultSubobject<UPDAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSetBase = CreateDefaultSubobject<UPDAttributeSetBase>(TEXT("AttributeSetBase"));
	InventoryComponent = CreateDefaultSubobject<UPDInventoryComponent>(TEXT("InventoryComponent"));
	
	TeamID = ETeamType::None;
}

UAbilitySystemComponent* APDPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APDPlayerState::SetDisplayName(const FString& NewDisplayName)
{
	const FString ResolvedDisplayName = NewDisplayName.IsEmpty() ? GetPlayerName() : NewDisplayName;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[LobbyPlayerState] SetDisplayName Requested=[%s] Stored=[%s] PlayerName=[%s] NetId=[%s]"),
		*NewDisplayName,
		*ResolvedDisplayName,
		*GetPlayerName(),
		*GetUniqueId().ToString());

	DisplayName = ResolvedDisplayName;
}

FString APDPlayerState::GetResolvedDisplayName() const
{
	return GetPlayerName().IsEmpty() ? DisplayName : GetPlayerName();
}

FBPUniqueNetId APDPlayerState::GetAvatarUniqueNetId() const
{
	FBPUniqueNetId Result;
	Result.SetUniqueNetId(GetUniqueId().GetUniqueNetId());
	return Result;
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

void APDPlayerState::OnRep_CharacterCustomInfo()
{

}

void APDPlayerState::OnRep_DisplayName()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[LobbyPlayerState] OnRep_DisplayName DisplayName=[%s] PlayerName=[%s] Resolved=[%s] NetId=[%s]"),
		*DisplayName,
		*GetPlayerName(),
		*GetResolvedDisplayName(),
		*GetUniqueId().ToString());
}

void APDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APDPlayerState, CharacterCustomInfo);
	DOREPLIFETIME(APDPlayerState, TeamID);
	DOREPLIFETIME(APDPlayerState, DisplayName);
}

void APDPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	APDPlayerState* NewPlayerState = Cast<APDPlayerState>(PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->TeamID = this->TeamID;
		NewPlayerState->DisplayName = this->DisplayName;
		NewPlayerState->CharacterCustomInfo = this->CharacterCustomInfo;
	}
}
