#include "Components/Combat/SkillManageComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "Pawn/PDPawnBase.h"
#include "Net/UnrealNetwork.h"
#include "PDGameplayTags.h"
#include "AbilitySystem/Abilities/Skill/GA_PlaceSpawnBase.h"
#include "PlayerState/PDPlayerState.h"
#include "Skill/SkillActor/PDSkillPlacementPreviewActor.h"


USkillManageComponent::USkillManageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

void USkillManageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsInPlacementMode)
	{
		return;
	}

	UpdatePlacementPreview();
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

int USkillManageComponent::BuySkill(TSubclassOf<UGameplayAbility> AbilityClass)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return INDEX_NONE;
	}

	if (!AbilityClass)
	{
		return INDEX_NONE;
	}
	
	RemoveExistingSkill(AbilityClass);
	
	if (Slots[0].IsEmpty())
	{
		ApplyAssign(0, AbilityClass);
		return 0;
	}

	if (Slots[1].IsEmpty())
	{
		ApplyAssign(1, AbilityClass);
		return 1;
	}

	return INDEX_NONE;
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

bool USkillManageComponent::BeginPlacement(UGA_PlaceSpawnBase* InSourceAbility,
	const FSpawnPlacementData& InPlacementData)
{
	if (!InSourceAbility || !InPlacementData.IsValidData())
	{
		return false;
	}

	if (bIsInPlacementMode)
	{
		EndPlacementInternal(true, true);
	}

	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	CurrentPlacementAbility = InSourceAbility;
	CurrentPlacementData = InPlacementData;
	bIsInPlacementMode = true;
	bCanPlaceCurrentLocation = false;

	if (UAbilitySystemComponent* ASC = OwnerPawn->GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(PDGameplayTags::Player_State_Placing);
	}

	if (OwnerPawn->IsLocallyControlled())
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters Params;
			Params.Owner = GetOwner();
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			PreviewActor = World->SpawnActor<APDSkillPlacementPreviewActor>(
				APDSkillPlacementPreviewActor::StaticClass(),
				FTransform::Identity,
				Params
			);

			if (PreviewActor)
			{
				PreviewActor->InitializePreview(
					CurrentPlacementData.PreviewStaticMesh,
					CurrentPlacementData.PreviewMaterial,
					CurrentPlacementData.PreviewScale
				);
			}
		}
	}

	SetComponentTickEnabled(true);
	UpdatePlacementPreview();
	return true;
}

void USkillManageComponent::ConfirmPlacement()
{
	if (!bIsInPlacementMode)
	{
		return;
	}

	if (!bCanPlaceCurrentLocation)
	{
		return;
	}

	Server_ConfirmPlacement(CurrentPlacementData, CachedPlacementTransform);
	EndPlacementInternal(false, true);
}

void USkillManageComponent::CancelPlacement()
{
	if (!bIsInPlacementMode)
	{
		return;
	}

	EndPlacementInternal(true, true);
}

void USkillManageComponent::CancelPlacementFromAbility(UGA_PlaceSpawnBase* InAbility)
{
	if (!bIsInPlacementMode)
	{
		return;
	}

	if (CurrentPlacementAbility != InAbility)
	{
		return;
	}

	EndPlacementInternal(true, false);
}

void USkillManageComponent::OnPlacementConfirmInput()
{
	ConfirmPlacement();
}

void USkillManageComponent::OnPlacementCancelInput()
{
	CancelPlacement();
}

void USkillManageComponent::EndPlacementInternal(bool bCancelled, bool bNotifyAbility)
{
	DestroyPreviewActor();

	SetComponentTickEnabled(false);
	bIsInPlacementMode = false;
	bCanPlaceCurrentLocation = false;
	CurrentPlacementData = FSpawnPlacementData{};
	CachedPlacementTransform = FTransform::Identity;

	if (APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = OwnerPawn->GetAbilitySystemComponent())
		{
			ASC->RemoveLooseGameplayTag(PDGameplayTags::Player_State_Placing);
		}
	}

	if (bNotifyAbility && CurrentPlacementAbility)
	{
		CurrentPlacementAbility->NotifyPlacementFinished(bCancelled);
	}

	CurrentPlacementAbility = nullptr;
}

void USkillManageComponent::DestroyPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void USkillManageComponent::UpdatePlacementPreview()
{
	if (!bIsInPlacementMode)
	{
		return;
	}

	FTransform NewTransform;
	bCanPlaceCurrentLocation = BuildPlacementTransform(CurrentPlacementData, NewTransform);
	CachedPlacementTransform = NewTransform;

	if (PreviewActor)
	{
		PreviewActor->SetPreviewTransform(CachedPlacementTransform);
		PreviewActor->SetPlacementValid(bCanPlaceCurrentLocation);
	}
}

bool USkillManageComponent::BuildPlacementTransform(const FSpawnPlacementData& InPlacementData,
	FTransform& OutTransform) const
{
	const APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	FVector ViewLocation = OwnerPawn->GetActorLocation();
	FRotator ViewRotation = OwnerPawn->GetActorRotation();

	if (const AController* OwnerController = OwnerPawn->GetController())
	{
		OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}

	const FVector Forward = ViewRotation.Vector().GetSafeNormal();
	const FVector TraceBaseLocation = OwnerPawn->GetActorLocation();

	const FVector TraceOrigin = TraceBaseLocation + Forward * InPlacementData.ForwardDistance;
	const FVector TraceStart = TraceOrigin + FVector(0.f, 0.f, InPlacementData.TraceStartHeight);
	const FVector TraceEnd = TraceOrigin - FVector(0.f, 0.f, InPlacementData.TraceDepth);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SkillPlacementTrace), false, OwnerPawn);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	if (InPlacementData.bRequireGroundHit && !bHit)
	{
		return false;
	}

	FVector FinalLocation = bHit ? Hit.ImpactPoint : TraceOrigin;
	FinalLocation += InPlacementData.SpawnLocationOffset;

	FRotator FinalRotation = InPlacementData.bUseOwnerYaw
		? FRotator(0.f, ViewRotation.Yaw, 0.f)
		: FRotator::ZeroRotator;

	FinalRotation += InPlacementData.SpawnRotationOffset;

	OutTransform = FTransform(FinalRotation, FinalLocation, InPlacementData.SpawnScale);
	return true;
}

void USkillManageComponent::Server_ConfirmPlacement_Implementation(const FSpawnPlacementData& InPlacementData, const FTransform& InConfirmedTransform)
{
	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->HasAuthority())
	{
		return;
	}

	const FTransform SpawnTransform = InConfirmedTransform;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerPawn;
	SpawnParams.Instigator = OwnerPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		InPlacementData.SpawnActorClass,
		SpawnTransform,
		SpawnParams
	);

	if (!SpawnedActor)
	{
		return;
	}

	ETeamType OwnerTeamID = ETeamType::None;
	if (const APDPlayerState* PS = OwnerPawn->GetPlayerState<APDPlayerState>())
	{
		OwnerTeamID = PS->GetTeamID();
	}

	if (APDPlacedSkillActorBase* PlacedActor = Cast<APDPlacedSkillActorBase>(SpawnedActor))
	{
		PlacedActor->InitializePlacedActor(InPlacementData.LifeTime, OwnerTeamID);
	}

	if (InPlacementData.bInitializeAsDamageableSkillActor)
	{
		if (APDDamageableSkillActor* DamageableActor = Cast<APDDamageableSkillActor>(SpawnedActor))
		{
			DamageableActor->InitializeShieldSettings(
				InPlacementData.MaxHealth,
				InPlacementData.SpawnStaticMesh,
				InPlacementData.SpawnBaseMaterial,
				OwnerTeamID,
				InPlacementData.DamageableType
			);
		}
	}

	SpawnedActor->SetActorScale3D(InPlacementData.SpawnScale);
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
