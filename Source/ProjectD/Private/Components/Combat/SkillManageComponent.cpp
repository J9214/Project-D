#include "Components/Combat/SkillManageComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "Pawn/PDPawnBase.h"
#include "Net/UnrealNetwork.h"
#include "PDGameplayTags.h"

bool FSkillSlot::IsEmpty() const
{
	return AbilityClass == nullptr;
}

USkillManageComponent::USkillManageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);

	SlotHandles[0] = FGameplayAbilitySpecHandle();
	SlotHandles[1] = FGameplayAbilitySpecHandle();
}

void USkillManageComponent::ServerRemoveSkillFromSlot_Implementation(FGameplayTag SlotTag)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}
	
	int32 SlotIndex = INDEX_NONE;
	if (SlotTag.MatchesTagExact(PDGameplayTags::InputTag_Ability_Skill1))
	{
		SlotIndex = 0;
	}
	else if (SlotTag.MatchesTagExact(PDGameplayTags::InputTag_Ability_Skill2))
	{
		SlotIndex = 1;
	}
	else
	{
		return;
	}
	
	if (SlotHandles[SlotIndex].IsValid())
	{
		ASC->CancelAbilityHandle(SlotHandles[SlotIndex]);
		ASC->ClearAbility(SlotHandles[SlotIndex]);
		SlotHandles[SlotIndex] = FGameplayAbilitySpecHandle();
	}
	
	Slots[SlotIndex] = FSkillSlot{};
}

void USkillManageComponent::ServerGrantSkillToSlot_Implementation(TSubclassOf<UGameplayAbility> AbilityClass, FGameplayTag SlotTag)
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

	int32 SlotIndex = INDEX_NONE;
	if (SlotTag.MatchesTagExact(PDGameplayTags::InputTag_Ability_Skill1))
	{
		SlotIndex = 0;
	}
	else if (SlotTag.MatchesTagExact(PDGameplayTags::InputTag_Ability_Skill2))
	{
		SlotIndex = 1;
	}
	else
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

	// Search for remaining possibilities and delete them
	{
		FGameplayTagContainer QueryTags;
		QueryTags.AddTag(SlotTag);

		TArray<FGameplayAbilitySpec*> Specs;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(QueryTags, Specs, false);

		for (FGameplayAbilitySpec* Spec : Specs)
		{
			if (!Spec)
			{
				continue;
			}

			ASC->CancelAbilityHandle(Spec->Handle);
			ASC->ClearAbility(Spec->Handle);
		}
	}

	if (!ASC->FindAbilitySpecFromClass(AbilityClass))
	{
		FGameplayAbilitySpec Spec(AbilityClass, 1);
		Spec.SourceObject = this;
		Spec.GetDynamicSpecSourceTags().AddTag(SlotTag);

		const FGameplayAbilitySpecHandle NewHandle = ASC->GiveAbility(Spec);
		if (!NewHandle.IsValid())
		{
			return;
		}

		SlotHandles[SlotIndex] = NewHandle;

		Slots[SlotIndex].AbilityClass = AbilityClass;
		Slots[SlotIndex].SlotTag = SlotTag;
	}
}

void USkillManageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(USkillManageComponent, Slots);
}

void USkillManageComponent::OnRep_Slots()
{
	UE_LOG(LogTemp, Warning, TEXT("USkillManageComponent::OnRep_Slots called"));
}

UAbilitySystemComponent* USkillManageComponent::GetASC() const
{
	if (const APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner()))
	{
		return OwnerPawn->GetAbilitySystemComponent();
	}

	return nullptr;
}
