#include "Components/Combat/SkillManageComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "Pawn/PDPawnBase.h"
#include "Net/UnrealNetwork.h"
#include "PDGameplayTags.h"

USkillManageComponent::USkillManageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);

	Slots.SetNum(2);
	SlotHandles.SetNum(2);
	
	SlotHandles[0] = FGameplayAbilitySpecHandle();
	SlotHandles[1] = FGameplayAbilitySpecHandle();
}

void USkillManageComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Slots[0].SlotTag = PDGameplayTags::InputTag_Ability_SkillQ;
	Slots[1].SlotTag = PDGameplayTags::InputTag_Ability_SkillE;
}

void USkillManageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(USkillManageComponent, Slots);
}

void USkillManageComponent::Server_BuySkill_Implementation(TSubclassOf<UGameplayAbility> AbilityClass)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (!AbilityClass)
	{
		return;
	}
	
	RemoveExistingSkill(AbilityClass);
	
	if (Slots[0].IsEmpty())
	{
		ApplyAssign(0, AbilityClass);
		return;
	}

	if (Slots[1].IsEmpty())
	{
		ApplyAssign(1, AbilityClass);
		return;
	}
}

void USkillManageComponent::Server_HandleSkillEquip_Implementation(const FSkillPayload& Payload, FGameplayTag ToSlotTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	const int32 ToIndex = GetSlotIndexByTag(ToSlotTag);
	if (ToIndex == INDEX_NONE)
	{
		return;
	}
	
	if (Payload.SourceType == ESkillDragSourceType::Slot)
	{
		const int32 FromIndex = GetSlotIndexByTag(Payload.FromSlotTag);
		if (FromIndex == INDEX_NONE || FromIndex == ToIndex)
		{
			return;
		}
		
		ApplyMoveOrSwap(FromIndex, ToIndex);
	}
	else
	{
		if (!Payload.AbilityClass)
		{
			return;
		}
		
		ApplyAssign(ToIndex, Payload.AbilityClass);
	}
}

void USkillManageComponent::Server_RemoveSkillFromSlot_Implementation(FGameplayTag SlotTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	const int32 SlotIndex = GetSlotIndexByTag(SlotTag);
	if (SlotIndex == INDEX_NONE)
	{
		return;
	}
	
	ApplyRemove(SlotIndex);
}

void USkillManageComponent::ApplyRemove(int32 SlotIndex)
{
	if (!IsValidOfSlotIndex(SlotIndex))
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}
	
	if (SlotHandles[SlotIndex].IsValid())
	{
		ASC->CancelAbilityHandle(SlotHandles[SlotIndex]);
		ASC->ClearAbility(SlotHandles[SlotIndex]);
		SlotHandles[SlotIndex] = FGameplayAbilitySpecHandle();
	}

	Slots[SlotIndex].AbilityClass = nullptr;
	Slots[SlotIndex].SlotTag = GetSlotTagFromIndex(SlotIndex);
}

void USkillManageComponent::ApplyAssign(int32 ToIndex, TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!IsValidOfSlotIndex(ToIndex))
	{
		return;
	}			
	
	RemoveExistingSkill(AbilityClass);
	ApplyRemove(ToIndex);

	if (!GiveAbilityToSlot(ToIndex, AbilityClass))
	{
		ApplyRemove(ToIndex);
	}
}

void USkillManageComponent::ApplyMoveOrSwap(int32 FromIndex, int32 ToIndex)
{
	if (!IsValidOfSlotIndex(FromIndex) || !IsValidOfSlotIndex(ToIndex) || FromIndex == ToIndex)
	{
		return;
	}
	
	if (Slots[FromIndex].IsEmpty())
	{
		return;
	}
	
	Swap(Slots[FromIndex].AbilityClass, Slots[ToIndex].AbilityClass);
	Swap(SlotHandles[FromIndex], SlotHandles[ToIndex]);
	
	Slots[0].SlotTag = GetSlotTagFromIndex(0);
	Slots[1].SlotTag = GetSlotTagFromIndex(1);
	
	SetSpecSlotTag(SlotHandles[0], GetSlotTagFromIndex(0));
	SetSpecSlotTag(SlotHandles[1], GetSlotTagFromIndex(1));
}

void USkillManageComponent::SetSpecSlotTag(const FGameplayAbilitySpecHandle& Handle, const FGameplayTag& NewSlotTag)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !Handle.IsValid())
	{
		return;
	}
	
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		return;
	}
	
	Spec->GetDynamicSpecSourceTags().RemoveTag(PDGameplayTags::InputTag_Ability_SkillQ);
	Spec->GetDynamicSpecSourceTags().RemoveTag(PDGameplayTags::InputTag_Ability_SkillE);
	Spec->GetDynamicSpecSourceTags().AddTag(NewSlotTag);

	ASC->MarkAbilitySpecDirty(*Spec);
}

bool USkillManageComponent::GiveAbilityToSlot(int32 SlotIndex, TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!IsValidOfSlotIndex(SlotIndex))
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !AbilityClass)
	{
		return false;
	}
	
	const FGameplayTag SlotTag = GetSlotTagFromIndex(SlotIndex);

	FGameplayAbilitySpec Spec(AbilityClass, 1);
	Spec.SourceObject = this;
	Spec.GetDynamicSpecSourceTags().AddTag(SlotTag);

	const FGameplayAbilitySpecHandle NewHandle = ASC->GiveAbility(Spec);
	if (!NewHandle.IsValid())
	{
		return false;
	}
	
	SlotHandles[SlotIndex] = NewHandle;
	Slots[SlotIndex].AbilityClass = AbilityClass;
	Slots[SlotIndex].SlotTag = SlotTag;

	return true;
}

void USkillManageComponent::RemoveExistingSkill(TSubclassOf<UGameplayAbility> AbilityClass)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !AbilityClass)
	{
		return;
	}
	
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].AbilityClass == AbilityClass)
		{
			ApplyRemove(i);
		}
	}
}

bool USkillManageComponent::IsValidOfSlotIndex(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= Slots.Num())
	{
		return false;
	}
	
	return true;
}

UAbilitySystemComponent* USkillManageComponent::GetASC() const
{
	if (const APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner()))
	{
		return OwnerPawn->GetAbilitySystemComponent();
	}

	return nullptr;
}

FSkillSlot USkillManageComponent::GetSlotByTag(FGameplayTag SlotTag) const
{
	const int32 SlotIndex = GetSlotIndexByTag(SlotTag);
	if (SlotIndex != INDEX_NONE)
	{
		return Slots[SlotIndex];
	}
	
	return FSkillSlot{};
}

int32 USkillManageComponent::GetSlotIndexByTag(FGameplayTag SlotTag) const
{
	if (SlotTag.MatchesTagExact(PDGameplayTags::InputTag_Ability_SkillQ))
	{
		return 0;
	}
	
	if (SlotTag.MatchesTagExact(PDGameplayTags::InputTag_Ability_SkillE))
	{
		return 1;
	}
	
	return INDEX_NONE;
}

FGameplayTag USkillManageComponent::GetSlotTagFromIndex(int32 SlotIndex) const
{
	if (SlotIndex == 0)
	{
		return PDGameplayTags::InputTag_Ability_SkillQ;
	}
	
	if (SlotIndex == 1)
	{
		return PDGameplayTags::InputTag_Ability_SkillE;
	}
	
	return FGameplayTag();
}

void USkillManageComponent::OnRep_Slots()
{
	UE_LOG(LogTemp, Warning, TEXT("USkillManageComponent::OnRep_Slots called"));
	OnSkillSlotsChanged.Broadcast();
}