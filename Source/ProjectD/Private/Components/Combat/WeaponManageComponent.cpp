#include "Components/Combat/WeaponManageComponent.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "Weapon/PDWeaponBase.h"
#include "Pawn/PDPawnBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/PDGameplayAbility.h"
#include "Structs/PDPlayerAbilitySet.h"
#include "Net/UnrealNetwork.h"

UWeaponManageComponent::UWeaponManageComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

    SetIsReplicatedByDefault(true);
    
    Slots.SetNum(2);
    BackSocketNames.SetNum(2);
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

    ApplyBuy(WeaponClass);
}

void UWeaponManageComponent::Server_HandleWeaponEquip_Implementation(const FWeaponPayload& Payload, int32 ToSlotIndex)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (!Slots.IsValidIndex(ToSlotIndex))
    {
        return;
    }

    if (Payload.SourceType == EWeaponDragSourceType::Slot)
    {
        const int32 FromIndex = Payload.FromSlotIndex;
        if (!Slots.IsValidIndex(FromIndex) || FromIndex == ToSlotIndex)
        {
            return;
        }

        ApplyMoveOrSwap(FromIndex, ToSlotIndex);
    }
    else
    {
        if (!Payload.WeaponClass)
        {
            return;
        }

        ApplyAssign(ToSlotIndex, Payload.WeaponClass);
    }
}

void UWeaponManageComponent::Server_RemoveWeaponFromSlot_Implementation(int32 SlotIndex)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (!Slots.IsValidIndex(SlotIndex))
    {
        return;
    }

    ApplyRemove(SlotIndex, true);
    
    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::ApplyBuy(TSubclassOf<APDWeaponBase> WeaponClass)
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

    EquipSlot(ToIndex);

    ScheduleRefreshAttachments();
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

void UWeaponManageComponent::EquipSlot(int32 SlotIndex)
{
    if (!Slots.IsValidIndex(SlotIndex))
    {
        return;
    }

    APDWeaponBase* NewWeapon = Slots[SlotIndex].WeaponActor;
    if (!IsValid(NewWeapon) || NewWeapon == EquippedWeapon)
    {
        return;
    }

    if (IsValid(EquippedWeapon))
    {
        UnequipCurrentWeapon();
    }

    EquippedWeapon = NewWeapon;
    EquippedSlotIndex = SlotIndex;

    //AttachToHand(EquippedWeapon);

    if (UDataAsset_Weapon* NewData = EquippedWeapon->WeaponData)
    {
        GrantAbilitiesFromWeaponData(NewData);
    }
}

void UWeaponManageComponent::UnequipCurrentWeapon()
{
    if (!IsValid(EquippedWeapon))
    {
        return;
    }

    RemoveCurrentWeaponGrantedAbilities();
    //AttachToBack(EquippedWeapon, EquippedSlotIndex);

    EquippedWeapon = nullptr;
    EquippedSlotIndex = INDEX_NONE;
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

    //AttachToBack(NewWeapon, SlotIndex);
    
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

APDWeaponBase* UWeaponManageComponent::GetWeaponInSlot(int32 SlotIndex) const
{
    return Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex].WeaponActor : nullptr;
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

void UWeaponManageComponent::OnRep_EquippedSlotIndex()
{
    ScheduleRefreshAttachments();
}

void UWeaponManageComponent::OnRep_EquippedWeapon()
{
    OnEquippedWeaponDataChanged.Broadcast(EquippedWeapon);
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
}
