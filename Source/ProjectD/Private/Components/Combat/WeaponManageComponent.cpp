#include "Components/Combat/WeaponManageComponent.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"
#include "Weapon/PDWeaponBase.h"
#include "Weapon/PDThrowableItemBase.h"
#include "Pawn/PDPawnBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/PDGameplayAbility.h"
#include "Structs/PDPlayerAbilitySet.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputSubsystems.h"

UWeaponManageComponent::UWeaponManageComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

    SetIsReplicatedByDefault(true);
    
    Slots.SetNum(2);
    BackSocketNames.SetNum(2);
    
    ThrowableSlots.SetNum(2);
    ThrowableBackSocketNames.SetNum(2);
    
    WeaponSlotCount = Slots.Num();
    ThrowableSlotCount = ThrowableSlots.Num();
    TotalSlotCount = WeaponSlotCount + ThrowableSlotCount;
}

void UWeaponManageComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWeaponManageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UWeaponManageComponent, Slots);
    DOREPLIFETIME(UWeaponManageComponent, EquippedWeapon);
    DOREPLIFETIME(UWeaponManageComponent, EquippedSlotIndex);
    DOREPLIFETIME(UWeaponManageComponent, ThrowableSlots);
    DOREPLIFETIME(UWeaponManageComponent, EquippedThrowable);
}

void UWeaponManageComponent::Server_BuyWeapon_Implementation(TSubclassOf<APDWeaponBase> WeaponClass)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (!WeaponClass)
    {
        return;
    }

    ApplyBuy_Weapon(WeaponClass);
}

void UWeaponManageComponent::Server_BuyThrowable_Implementation(TSubclassOf<APDThrowableItemBase> ThrowableItemClass)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (!ThrowableItemClass)
    {
        return;
    }

    ApplyBuy_Throwable(ThrowableItemClass);
}

void UWeaponManageComponent::Server_HandleWeaponEquip_Implementation(const FEquipmentPayload& Payload, int32 ToSlotIndex)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (ToSlotIndex < 0 || ToSlotIndex >= TotalSlotCount)
    {
        return;
    }
    
    if (Payload.SourceType == EWeaponDragSourceType::Slot)
    {
        const int32 FromIndex = Payload.FromSlotIndex;
        if (FromIndex < 0 || FromIndex >= TotalSlotCount || FromIndex == ToSlotIndex)
        {
            return;
        }

        const bool bFromWeapon = IsWeaponSlotIndex(FromIndex);
        const bool bToWeapon = IsWeaponSlotIndex(ToSlotIndex);
        const bool bFromThrow = IsThrowableSlotIndex(FromIndex);
        const bool bToThrow = IsThrowableSlotIndex(ToSlotIndex);
        if (!(bFromWeapon && bToWeapon) && !(bFromThrow && bToThrow))
        {
            return;
        }
        
        ApplyMoveOrSwap_Global(FromIndex, ToSlotIndex);
    }
    else
    {
        if (IsWeaponSlotIndex(ToSlotIndex))
        {
            if (!Payload.WeaponClass)
            {
                return;
            }
            
            ApplyAssign(ToSlotIndex, Payload.WeaponClass);
        }
        else if (IsThrowableSlotIndex(ToSlotIndex))
        {
            if (!Payload.ThrowableItemClass)
            {
                return;
            }
            
            ApplyAssign_Throwable(ToThrowableLocalIndex(ToSlotIndex), Payload.ThrowableItemClass);
        }
    }
}

void UWeaponManageComponent::Server_RemoveWeaponFromSlot_Implementation(int32 SlotIndex)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (SlotIndex < 0 || SlotIndex >= TotalSlotCount)
    {
        return;
    }

    ApplyRemove_Global(SlotIndex, true);
    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::ApplyBuy_Weapon(TSubclassOf<APDWeaponBase> WeaponClass)
{
    const int32 EmptyIndex = FindFirstEmptySlot();
    if (EmptyIndex == INDEX_NONE || !WeaponClass)
    {
        return;
    }

    if (!SpawnAndPlace(EmptyIndex, WeaponClass))
    {
        return;
    }

    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::ApplyBuy_Throwable(TSubclassOf<APDThrowableItemBase> ThrowableItemClass)
{
    const int32 EmptyIndex = FindFirstEmptyThrowableSlot();
    if (EmptyIndex == INDEX_NONE || !ThrowableItemClass)
    {
        return;
    }

    if (!SpawnAndPlace_Throwable(EmptyIndex, ThrowableItemClass))
    {
        return;
    }

    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::ApplyAssign(int32 ToIndex, TSubclassOf<APDWeaponBase> WeaponClass)
{
    if (!Slots.IsValidIndex(ToIndex) || !WeaponClass)
    {
        return;
    }

    ApplyRemove(ToIndex, true);

    if (!SpawnAndPlace(ToIndex, WeaponClass))
    {
        return;
    }

    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::ApplyAssign_Throwable(int32 ToIndex, TSubclassOf<APDThrowableItemBase> ThrowableItemClass)
{
    if (!ThrowableSlots.IsValidIndex(ToIndex) || !ThrowableItemClass)
    {
        return;
    }

    ApplyRemove_Throwable(ToIndex, true);

    if (!SpawnAndPlace_Throwable(ToIndex, ThrowableItemClass))
    {
        return;
    }

    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::ApplyMoveOrSwap_Global(int32 FromIndex, int32 ToIndex)
{
    if (IsWeaponSlotIndex(FromIndex) && IsWeaponSlotIndex(ToIndex))
    {
        ApplyMoveOrSwap(FromIndex, ToIndex);
    }
    else if (IsThrowableSlotIndex(FromIndex) && IsThrowableSlotIndex(ToIndex))
    {
        const int32 FromLocal = ToThrowableLocalIndex(FromIndex);
        const int32 ToLocal   = ToThrowableLocalIndex(ToIndex);
        ApplyMoveOrSwap_Throwable(FromLocal, ToLocal);
    }
}

void UWeaponManageComponent::ApplyMoveOrSwap(int32 FromIndex, int32 ToIndex)
{
    if (!Slots.IsValidIndex(FromIndex) || !Slots.IsValidIndex(ToIndex) || FromIndex == ToIndex)
    {
        return;
    }

    APDWeaponBase* FromWeapon = Slots[FromIndex].WeaponActor;
    if (!IsValid(FromWeapon))
    {
        return;
    }

    APDWeaponBase* ToWeapon = Slots[ToIndex].WeaponActor;

    Slots[FromIndex].WeaponActor = ToWeapon;
    Slots[ToIndex].WeaponActor = FromWeapon;

    if (IsValid(EquippedWeapon))
    {
        EquippedSlotIndex = FindSlotIndexByWeapon(EquippedWeapon);
    }

    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::ApplyMoveOrSwap_Throwable(int32 FromIndex, int32 ToIndex)
{
    if (!ThrowableSlots.IsValidIndex(FromIndex) || !ThrowableSlots.IsValidIndex(ToIndex) || FromIndex == ToIndex)
    {
        return;
    }

    APDThrowableItemBase* FromItem = ThrowableSlots[FromIndex].ThrowableItemActor;
    if (!IsValid(FromItem))
    {
        return;
    }

    APDThrowableItemBase* ToItem = ThrowableSlots[ToIndex].ThrowableItemActor;

    ThrowableSlots[FromIndex].ThrowableItemActor = ToItem;
    ThrowableSlots[ToIndex].ThrowableItemActor = FromItem;

    if (IsThrowableSlotIndex(EquippedSlotIndex))
    {
        const int32 EquippedLocal = ToThrowableLocalIndex(EquippedSlotIndex);
        if (EquippedLocal == FromIndex)
        {
            EquippedSlotIndex = WeaponSlotCount + ToIndex;
        }
        else if (EquippedLocal == ToIndex)
        {
            EquippedSlotIndex = WeaponSlotCount + FromIndex;
        }
    }

    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::ApplyRemove_Global(int32 SlotIndex, bool bDestroy)
{
    if (IsWeaponSlotIndex(SlotIndex))
    {
        ApplyRemove(SlotIndex, bDestroy);
    }
    else if (IsThrowableSlotIndex(SlotIndex))
    {
        ApplyRemove_Throwable(ToThrowableLocalIndex(SlotIndex), bDestroy);
    }
}

void UWeaponManageComponent::ApplyRemove(int32 SlotIndex, bool bDestroyActor)
{
    if (!Slots.IsValidIndex(SlotIndex))
    {
        return;
    }

    APDWeaponBase* Old = Slots[SlotIndex].WeaponActor;
    if (!IsValid(Old))
    {
        Slots[SlotIndex].WeaponActor = nullptr;
        return;
    }

    if (EquippedWeapon == Old)
    {
        UnequipCurrentWeapon();
    }

    if (bDestroyActor)
    {
        Old->Destroy();
    }

    Slots[SlotIndex].WeaponActor = nullptr;
}

void UWeaponManageComponent::ApplyRemove_Throwable(int32 SlotIndex, bool bDestroy)
{
    if (!ThrowableSlots.IsValidIndex(SlotIndex))
        return;

    APDThrowableItemBase* Old = ThrowableSlots[SlotIndex].ThrowableItemActor;
    if (!IsValid(Old))
    {
        ThrowableSlots[SlotIndex].ThrowableItemActor = nullptr;
        return;
    }

    const int32 ThrowableIndex = WeaponSlotCount + SlotIndex;
    if (EquippedSlotIndex == ThrowableIndex)
    {
        UnequipCurrentWeapon();
    }

    if (bDestroy)
    {
        Old->Destroy();
    }

    ThrowableSlots[SlotIndex].ThrowableItemActor = nullptr;
}

void UWeaponManageComponent::EquipSlot(int32 SlotIndex)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (SlotIndex < 0 || SlotIndex >= TotalSlotCount)
    {
        return;
    }
    
    if (EquippedSlotIndex == SlotIndex)
    {
        return;
    }
    
    UnequipCurrentWeapon();
    
    if (IsWeaponSlotIndex(SlotIndex))
    {
        APDWeaponBase* NewWeapon = Slots[SlotIndex].WeaponActor;
        if (!IsValid(NewWeapon) || NewWeapon == EquippedWeapon)
        {
            return;
        }

        EquippedWeapon = NewWeapon;
        EquippedThrowable = nullptr;
        EquippedSlotIndex = SlotIndex;
        
        ScheduleRefreshAttachments();
    }
    else if (IsThrowableSlotIndex(SlotIndex))
    {
        APDThrowableItemBase* NewThrowable = ThrowableSlots[ToThrowableLocalIndex(SlotIndex)].ThrowableItemActor;
        if (!IsValid(NewThrowable) || NewThrowable == EquippedThrowable)
        {
            return;
        }

        EquippedThrowable = NewThrowable;
        EquippedWeapon = nullptr;
        EquippedSlotIndex = SlotIndex;
        
        ScheduleRefreshAttachments();
    }
    
    RefreshEquipIMC();
}

void UWeaponManageComponent::UnequipCurrentWeapon()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (EquippedSlotIndex == INDEX_NONE)
    {
        return;
    }

    EquippedWeapon = nullptr;
    EquippedThrowable = nullptr;
    EquippedSlotIndex = INDEX_NONE;

    ScheduleRefreshAttachments();
    RefreshEquipIMC();
}

APDWeaponBase* UWeaponManageComponent::SpawnWeaponActor(TSubclassOf<APDWeaponBase> WeaponClass)
{
    APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
    if (!OwnerPawn || !OwnerPawn->GetWorld())
    {
        return nullptr;
    }

    if (!WeaponClass)
    {
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.Owner = OwnerPawn;
    Params.Instigator = OwnerPawn;

    return OwnerPawn->GetWorld()->SpawnActor<APDWeaponBase>(WeaponClass, Params);
}

APDThrowableItemBase* UWeaponManageComponent::SpawnThrowableItemActor(TSubclassOf<APDThrowableItemBase> ThrowableItemClass)
{
    APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
    if (!OwnerPawn || !OwnerPawn->GetWorld())
    {
        return nullptr;
    }

    if (!ThrowableItemClass)
    {
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.Owner = OwnerPawn;
    Params.Instigator = OwnerPawn;

    return OwnerPawn->GetWorld()->SpawnActor<APDThrowableItemBase>(ThrowableItemClass, Params);
}

bool UWeaponManageComponent::SpawnAndPlace(int32 SlotIndex, TSubclassOf<APDWeaponBase> WeaponClass)
{
    if (!Slots.IsValidIndex(SlotIndex) || !WeaponClass)
    {
        return false;
    }

    APDWeaponBase* NewWeapon = SpawnWeaponActor(WeaponClass);
    if (!IsValid(NewWeapon))
    {
        return false;
    }

    NewWeapon->InitWeaponData();
    Slots[SlotIndex].WeaponActor = NewWeapon;

    return true;
}

bool UWeaponManageComponent::SpawnAndPlace_Throwable(int32 SlotIndex, TSubclassOf<APDThrowableItemBase> ThrowableItemClass)
{
    if (!ThrowableSlots.IsValidIndex(SlotIndex) || !ThrowableItemClass)
    {
        return false;
    }

    APDThrowableItemBase* NewItem = SpawnThrowableItemActor(ThrowableItemClass);
    if (!IsValid(NewItem))
    {
        return false;
    }

    ThrowableSlots[SlotIndex].ThrowableItemActor = NewItem;
    
    return true;
}

void UWeaponManageComponent::AttachToHand(APDWeaponBase* Weapon)
{
    APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
    if (!OwnerPawn || !Weapon || !Weapon->WeaponData)
    {
        return;
    }

    Weapon->AttachToComponent(
        OwnerPawn->GetSkeletalMeshComponent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        HandSocketName
    );
}

void UWeaponManageComponent::AttachToBack(APDWeaponBase* Weapon, int32 SlotIndex)
{
    APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
    if (!OwnerPawn || !Weapon || !Weapon->WeaponData)
    {
        return;
    }

    FName BackSocket = NAME_None;
    if (BackSocketNames.IsValidIndex(SlotIndex))
    {
        BackSocket = BackSocketNames[SlotIndex];
    }

    Weapon->AttachToComponent(
        OwnerPawn->GetSkeletalMeshComponent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        BackSocket
    );
}

void UWeaponManageComponent::ThrowableAttachToHand(APDThrowableItemBase* Throwable)
{
    APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }

    Throwable->AttachToComponent(
        OwnerPawn->GetSkeletalMeshComponent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        ThrowableHandSocketName
    );
}

void UWeaponManageComponent::ThrowableAttachToBack(APDThrowableItemBase* Throwable, int32 SlotIndex)
{
    APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner());
    if (!OwnerPawn)
    {
        return;
    }

    FName BackSocket = NAME_None;
    if (ThrowableBackSocketNames.IsValidIndex(SlotIndex))
    {
        BackSocket = ThrowableBackSocketNames[SlotIndex];
    }

    Throwable->AttachToComponent(
        OwnerPawn->GetSkeletalMeshComponent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        BackSocket
    );
}

APDWeaponBase* UWeaponManageComponent::GetWeaponInSlot(int32 SlotIndex) const
{
    return Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex].WeaponActor : nullptr;
}

APDThrowableItemBase* UWeaponManageComponent::GetThrowableInSlot(int32 SlotIndex) const
{
    const int32 LocalIndex = ToThrowableLocalIndex(SlotIndex);
    return ThrowableSlots.IsValidIndex(LocalIndex) ? ThrowableSlots[LocalIndex].ThrowableItemActor : nullptr;
}

int32 UWeaponManageComponent::FindNextThrowableGlobalSlotIndex() const
{
    if (ThrowableSlotCount <= 0)
    {
        return INDEX_NONE;
    }

    int32 StartLocalIndex = 0;
    
    if (IsThrowableSlotIndex(EquippedSlotIndex))
    {
        const int32 CurLocalIndex = ToThrowableLocalIndex(EquippedSlotIndex);
        StartLocalIndex = (CurLocalIndex + 1) % ThrowableSlotCount;
    }

    for (int32 i = 0; i < ThrowableSlotCount; ++i)
    {
        const int32 LocalIndex = (StartLocalIndex + i) % ThrowableSlotCount;
        const int32 GlobalIndex = WeaponSlotCount + LocalIndex;

        if (HasItemInSlot(GlobalIndex))
        {
            return GlobalIndex;
        }
    }

    return INDEX_NONE;
}

bool UWeaponManageComponent::TryGetMontageEntry(int32 SlotIndex, EPDWeaponMontageAction Action, FPDWeaponMontageEntry& OutEntry) const
{
    OutEntry = FPDWeaponMontageEntry();
    
    if (IsWeaponSlotIndex(SlotIndex))
    {
        const APDWeaponBase* Weapon = GetWeaponInSlot(SlotIndex);
        if (!IsValid(Weapon) || !Weapon->WeaponData)
        {
            return false;
        }
        
        const auto& Entry = Weapon->WeaponData->WeaponMontages.Get(Action);
        OutEntry = Entry;
        
        return true;
    }
    
    if (IsThrowableSlotIndex(SlotIndex))
    {
        const APDThrowableItemBase* Throwable = GetThrowableInSlot(SlotIndex);
        if (!IsValid(Throwable) || !Throwable->GetThrowableData())
        {
            return false;
        }
        
        const auto& Entry = Throwable->GetThrowableData()->ThrowableMontages.Get(Action);
        OutEntry = Entry;
        
        return true;
    }

    return false;
}

bool UWeaponManageComponent::HasItemInSlot(int32 SlotIndex) const
{
    if (IsWeaponSlotIndex(SlotIndex))
    {
        return IsValid(GetWeaponInSlot(SlotIndex));
    }
    
    if (IsThrowableSlotIndex(SlotIndex))
    {
        const int32 LocalIndex = ToThrowableLocalIndex(SlotIndex);
        return ThrowableSlots.IsValidIndex(LocalIndex) && IsValid(ThrowableSlots[LocalIndex].ThrowableItemActor);
    }
    
    return false;
}

UInputMappingContext* UWeaponManageComponent::GetEquippedIMC(int32& OutPriority) const
{
    if (IsWeaponSlotIndex(EquippedSlotIndex))
    {
        if (APDWeaponBase* Weapon = EquippedWeapon)
        {
            if (UDataAsset_Weapon* DA = Weapon->WeaponData)
            {
                OutPriority = DA->IMCPriority;
                return DA->WeaponIMC;
            }
        }
    }

    if (IsThrowableSlotIndex(EquippedSlotIndex))
    {
        if (APDThrowableItemBase* Item = EquippedThrowable)
        {
            if (UDataAsset_Throwable* DA = Item->GetThrowableData())
            {
                OutPriority = DA->IMCPriority;
                return DA->ThrowableIMC;
            }
        }
    }

    return nullptr;
}

int32 UWeaponManageComponent::FindFirstEmptySlot() const
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (!IsValid(Slots[i].WeaponActor))
        {
            return i;
        }
    }

    return INDEX_NONE;
}

int32 UWeaponManageComponent::FindFirstEmptyThrowableSlot() const
{
    for (int32 i = 0; i < ThrowableSlots.Num(); ++i)
    {
        if (!IsValid(ThrowableSlots[i].ThrowableItemActor))
        {
            return i;
        }
    }
    
    return INDEX_NONE;
}

int32 UWeaponManageComponent::FindSlotIndexByWeapon(APDWeaponBase* Weapon) const
{
    if (!IsValid(Weapon))
    {
        return INDEX_NONE;
    }

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].WeaponActor == Weapon)
        {
            return i;
        }
    }

    return INDEX_NONE;
}

UEnhancedInputLocalPlayerSubsystem* UWeaponManageComponent::GetEnhancedInputSubsystem(APawn* Pawn)
{
    if (!Pawn)
    {
        return nullptr;
    }
    
    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC || !PC->IsLocalController())
    {
        return nullptr;
    }
    
    ULocalPlayer* LP = PC->GetLocalPlayer();
    if (!LP)
    {
        return nullptr;
    }
    
    return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP);
}

void UWeaponManageComponent::RefreshEquipIMC()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    UEnhancedInputLocalPlayerSubsystem* Subsys = GetEnhancedInputSubsystem(OwnerPawn);
    if (!Subsys)
    {
        return;
    }
    
    if (CurrentAppliedIMC)
    {
        Subsys->RemoveMappingContext(CurrentAppliedIMC);
        CurrentAppliedIMC = nullptr;
    }
    
    int32 NewIMCPriority = 0;
    if (UInputMappingContext* NewIMC = GetEquippedIMC(NewIMCPriority))
    {
        Subsys->AddMappingContext(NewIMC, NewIMCPriority);
        CurrentAppliedIMC = NewIMC;
        CurrentAppliedIMCPriority = NewIMCPriority;
    }
}

UAbilitySystemComponent* UWeaponManageComponent::GetASC() const
{
    if (const APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetOwner()))
    {
        return OwnerPawn->GetAbilitySystemComponent();
    }

    return nullptr;
}

void UWeaponManageComponent::GrantAbilitiesFromWeaponData(UDataAsset_Weapon* WeaponData)
{
    UAbilitySystemComponent* ASC = GetASC();
    if (!ASC || !WeaponData)
    {
        return;
    }

    for (const FPDPlayerAbilitySet& Entry : WeaponData->GrantedAbilitySets)
    {
        if (!Entry.IsValid())
        {
            continue;
        }

        FGameplayAbilitySpec Spec(Entry.AbilityToGrant);
        Spec.SourceObject = GetOwner();
        Spec.Level = 1;
        Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);

        const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
        CurrentWeaponGrantedAbilityHandles.Add(Handle);
    }
}

void UWeaponManageComponent::RemoveCurrentWeaponGrantedAbilities()
{
    UAbilitySystemComponent* ASC = GetASC();
    if (!ASC)
    {
        CurrentWeaponGrantedAbilityHandles.Reset();
        return;
    }

    for (const FGameplayAbilitySpecHandle& Handle : CurrentWeaponGrantedAbilityHandles)
    {
        if (Handle.IsValid())
        {
            ASC->CancelAbilityHandle(Handle);
            ASC->ClearAbility(Handle);
        }
    }

    CurrentWeaponGrantedAbilityHandles.Reset();
}

void UWeaponManageComponent::ScheduleRefreshAttachments()
{
    if (bPendingRefreshAttachments)
    {
        return;
    }
    bPendingRefreshAttachments = true;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UWeaponManageComponent::DoRefreshAttachments);
}

void UWeaponManageComponent::OnRep_Slots()
{
    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::OnRep_ThrowableSlots()
{
    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::OnRep_EquippedSlotIndex()
{
    ScheduleRefreshAttachments();
    RefreshEquipIMC();
}

void UWeaponManageComponent::OnRep_EquippedWeapon()
{
    OnEquippedWeaponChanged.Broadcast(EquippedWeapon);
}

void UWeaponManageComponent::OnRep_EquippedThrowable()
{
    OnEquippedThrowableChanged.Broadcast(EquippedThrowable);
}

void UWeaponManageComponent::DoRefreshAttachments()
{
    bPendingRefreshAttachments = false;
    RefreshAttachments();
}

void UWeaponManageComponent::RefreshAttachments()
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        APDWeaponBase* Weapon = Slots[i].WeaponActor;
        if (!IsValid(Weapon))
        {
            continue;
        }

        if (i == EquippedSlotIndex)
        {
            AttachToHand(Weapon);
        }
        else
        {
            AttachToBack(Weapon, i);
        }
    }

    const bool bThrowableEquipped = IsThrowableSlotIndex(EquippedSlotIndex);
    const int32 EquippedThrowableIndex = bThrowableEquipped ? ToThrowableLocalIndex(EquippedSlotIndex) : INDEX_NONE;
    
    for (int32 i = 0; i < ThrowableSlots.Num(); ++i)
    {
        APDThrowableItemBase* Throwable = ThrowableSlots[i].ThrowableItemActor;
        if (!IsValid(Throwable))
        {
            continue;
        }

        if (i == EquippedThrowableIndex)
        {
            ThrowableAttachToHand(Throwable);
        }
        else
        {
            ThrowableAttachToBack(Throwable, i);
        }
    }
}

void UWeaponManageComponent::ChangeWeapon(int32 TargetSlot, TSubclassOf<APDWeaponBase> NewWeaponClass)
{
    //if (!(GetOwnerRole() == ROLE_Authority) || !Slots.IsValidIndex(TargetSlot))
    //{
    //    return;
    //}

    if (IsValid(Slots[TargetSlot].WeaponActor))
    {
        if (TargetSlot == EquippedSlotIndex)
        {
            UnequipCurrentWeapon();
        }

        Slots[TargetSlot].WeaponActor->Destroy();
        Slots[TargetSlot].WeaponActor = nullptr;
    }

    APDWeaponBase* NewWeapon = SpawnWeaponActor(NewWeaponClass);
    if (IsValid(NewWeapon))
    {
        NewWeapon->InitWeaponData();
        Slots[TargetSlot].WeaponActor = NewWeapon;
        AttachToBack(NewWeapon, TargetSlot);
    }
}