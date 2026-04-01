#include "PlayerState/PDPlayerState.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "Net/UnrealNetwork.h"
#include "Components/Inventory/PDInventoryComponent.h"
#include <Controller/PDPlayerController.h>
#include <GameMode/PDGameModeBase.h>
#include <Controller/PDLobbyPlayerController.h>

namespace
{
FString DescribeCharacterCustomInfo(const FPDCharacterCustomInfo& CharacterInfo)
{
	FString Summary = FString::Printf(
		TEXT("CharacterId=%d Enum=%d Float=%d Color=%d Bool=%d"),
		CharacterInfo.CharacterId,
		CharacterInfo.EnumParameters.Num(),
		CharacterInfo.FloatParameters.Num(),
		CharacterInfo.ColorParameters.Num(),
		CharacterInfo.BoolParameters.Num());

	for (int32 Index = 0; Index < CharacterInfo.EnumParameters.Num(); ++Index)
	{
		const FPDMutableEnumParameter& Param = CharacterInfo.EnumParameters[Index];
		Summary += FString::Printf(TEXT(" | Enum[%d]={Name='%s',Option='%s',Range=%d}"), Index, *Param.ParameterName, *Param.SelectedOptionName, Param.RangeIndex);
	}

	return Summary;
}
}

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

void APDPlayerState::SetCharacterCustomInfo(const FPDCharacterCustomInfo& NewCharacterCustomInfo)
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[CharacterCustomizationFlow][PlayerState::SetCharacterCustomInfo] PS=%s NetId=[%s] %s"),
		*GetNameSafe(this),
		*GetUniqueId().ToString(),
		*DescribeCharacterCustomInfo(NewCharacterCustomInfo));
	CharacterCustomInfo = NewCharacterCustomInfo;
	HandleCharacterCustomInfoChanged();
	ForceNetUpdate();
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
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[CharacterCustomizationFlow][PlayerState::OnRep_CharacterCustomInfo] PS=%s NetId=[%s] %s"),
		*GetNameSafe(this),
		*GetUniqueId().ToString(),
		*DescribeCharacterCustomInfo(CharacterCustomInfo));
	HandleCharacterCustomInfoChanged();

	if (APDLobbyPlayerController* LocalPC = Cast<APDLobbyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		LocalPC->RefreshLocalLobbyUI();
	}
}

void APDPlayerState::HandleCharacterCustomInfoChanged()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[CharacterCustomizationFlow][PlayerState::HandleCharacterCustomInfoChanged] PS=%s NetId=[%s] %s"),
		*GetNameSafe(this),
		*GetUniqueId().ToString(),
		*DescribeCharacterCustomInfo(CharacterCustomInfo));
	CharacterCustomInfoChangedNative.Broadcast(CharacterCustomInfo);
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

	if (APDLobbyPlayerController* LocalPC = Cast<APDLobbyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		LocalPC->RefreshLocalLobbyUI();
	}
}

void APDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(APDPlayerState, CharacterCustomInfo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(APDPlayerState, TeamID);
	DOREPLIFETIME(APDPlayerState, DisplayName); 
	DOREPLIFETIME(APDPlayerState, bClientReady);
}

void APDPlayerState::Server_SetReady_Implementation()
{
	if (bClientReady)
	{
		return;
	}

	bClientReady = true;

	if (APDGameModeBase* GM = GetWorld()->GetAuthGameMode<APDGameModeBase>())
	{
		GM->CheckAllPlayersReady();
	}
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
		NewPlayerState->HandleCharacterCustomInfoChanged();
	}
}
