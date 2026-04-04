#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "Structs/SpawnPlacementData.h"
#include "SkillManageComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnSkillSlotsChanged);

class UGameplayAbility;
class UAbilitySystemComponent;
class UGA_PlaceSpawnBase;
class APDSkillPlacementPreviewActor;
class UAbilitySystemComponent;


UENUM(BlueprintType)
enum class ESkillDragSourceType : uint8
{
	Slot,
	External
};

USTRUCT(BlueprintType)
struct FSkillPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillDragSourceType SourceType = ESkillDragSourceType::Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag FromSlotTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;
};

USTRUCT(BlueprintType)
struct FSkillSlot
{
	GENERATED_BODY()

public:
	FORCEINLINE bool IsEmpty() const { return AbilityClass == nullptr; }
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SlotTag;
};

UCLASS()
class PROJECTD_API USkillManageComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:
	USkillManageComponent();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Skill")
	void Server_BuySkill(TSubclassOf<UGameplayAbility> AbilityClass);

	int BuySkill(TSubclassOf<UGameplayAbility> AbilityClass);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Skill")
	void Server_HandleSkillEquip(const FSkillPayload& Payload, FGameplayTag ToSlotTag);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Skill")
	void Server_RemoveSkillFromSlot(FGameplayTag SlotTag);

	UFUNCTION(BlueprintCallable, Category="Skill")
	void RemoveAllSkillsOnDeath();
	
	UFUNCTION(BlueprintCallable, Category="Skill")
	FSkillSlot GetSlotByTag(FGameplayTag SlotTag) const;

	FGameplayTag GetSlotTagFromIndex(int32 SlotIndex) const;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	
	UPROPERTY()
	TObjectPtr<APDSkillPlacementPreviewActor> PreviewActor = nullptr;

	UPROPERTY()
	TObjectPtr<UGA_PlaceSpawnBase> CurrentPlacementAbility = nullptr;

	UPROPERTY()
	FSpawnPlacementData CurrentPlacementData;

	UPROPERTY()
	bool bIsInPlacementMode = false;

	UPROPERTY()
	bool bCanPlaceCurrentLocation = false;

	UPROPERTY()
	FTransform CachedPlacementTransform;

private:
	UFUNCTION()
	void OnRep_Slots();
	
	void ApplyRemove(int32 SlotIndex);
	void ApplyAssign(int32 ToIndex, TSubclassOf<UGameplayAbility> AbilityClass);
	void ApplyMoveOrSwap(int32 FromIndex, int32 ToIndex);

	void SetSpecSlotTag(const FGameplayAbilitySpecHandle& Handle, const FGameplayTag& NewSlotTag);
	
	int32 GetSlotIndexByTag(FGameplayTag SlotTag) const;
	UAbilitySystemComponent* GetASC() const;

	bool GiveAbilityToSlot(int32 SlotIndex, TSubclassOf<UGameplayAbility> AbilityClass);

	void RemoveExistingSkill(TSubclassOf<UGameplayAbility> AbilityClass);
	
	bool IsValidOfSlotIndex(int32 SlotIndex) const;
	
public:
	FOnSkillSlotsChanged OnSkillSlotsChanged;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_Slots)
	TArray<FSkillSlot> Slots;

	TArray<FGameplayAbilitySpecHandle> SlotHandles;
	
public:
	bool BeginPlacement(UGA_PlaceSpawnBase* InSourceAbility, const FSpawnPlacementData& InPlacementData);

	void ConfirmPlacement();
	void CancelPlacement();
	void CancelPlacementFromAbility(UGA_PlaceSpawnBase* InAbility);

	UFUNCTION(BlueprintPure)
	bool IsInPlacementMode() const { return bIsInPlacementMode; }

	UFUNCTION(BlueprintCallable)
	void OnPlacementConfirmInput();

	UFUNCTION(BlueprintCallable)
	void OnPlacementCancelInput();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EndPlacementInternal(bool bCancelled, bool bNotifyAbility);
	void DestroyPreviewActor();
	void UpdatePlacementPreview();
	bool BuildPlacementTransform(const FSpawnPlacementData& InPlacementData, FTransform& OutTransform) const;

	UFUNCTION(Server, Reliable)
	void Server_ConfirmPlacement(const FSpawnPlacementData& InPlacementData, const FTransform& InConfirmedTransform);
};
