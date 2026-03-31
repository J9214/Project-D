#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "BlueprintDataDefinitions.h"
#include "GameInstance/PDCharacterCustomInfo.h"
#include "Interface/PDTeamInterface.h"
#include "MuCO/CustomizableObjectInstance.h"
#include "PDPlayerState.generated.h"

class UPDAbilitySystemComponent;
class UPDAttributeSetBase;
class UGameplayEffect;
class UPDInventoryComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FPDOnCharacterCustomInfoChangedNative, const FPDCharacterCustomInfo&);

UCLASS()
class PROJECTD_API APDPlayerState : public APlayerState, public IAbilitySystemInterface, public IPDTeamInterface
{
	GENERATED_BODY()
	
public:
	APDPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	FORCEINLINE UPDInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	virtual ETeamType GetTeamID() const override { return TeamID; }
	
	UFUNCTION(BlueprintPure)
	UPDAbilitySystemComponent* GetPDAbilitySystemComponent() const { return AbilitySystemComponent; }
	UPDAttributeSetBase* GetPDAttributeSetBase() const { return AttributeSetBase; }

	void InitAbilityActorInfo(AActor* AvatarActor);

	void SetDeadState();

	void SetReviveState();

	void SetTeamID(ETeamType NewTeamID) { TeamID = NewTeamID; }

	UPROPERTY(ReplicatedUsing = OnRep_CharacterCustomInfo, VisibleAnywhere, BlueprintReadOnly, Category = "Custom")
	FPDCharacterCustomInfo CharacterCustomInfo;

	UFUNCTION(BlueprintCallable, Category = "Custom")
	void SetCharacterCustomInfo(const FPDCharacterCustomInfo& NewCharacterCustomInfo);

	UFUNCTION(BlueprintPure, Category = "Custom")
	const FPDCharacterCustomInfo& GetCharacterCustomInfo() const { return CharacterCustomInfo; }

	UFUNCTION(BlueprintCallable)
	void SetDisplayName(const FString& NewDisplayName);
	const FString& GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure)
	FString GetResolvedDisplayName() const;

	UFUNCTION(BlueprintPure)
	FBPUniqueNetId GetAvatarUniqueNetId() const;

	UFUNCTION()
	void OnRep_CharacterCustomInfo();

	UFUNCTION(BlueprintImplementableEvent, Category = "Custom")
	void BP_OnCharacterCustomInfoChanged(const FPDCharacterCustomInfo& NewCharacterCustomInfo);

	UFUNCTION(BlueprintCallable, Category = "Custom")
	void SetCustomizableObjectInstance(UCustomizableObjectInstance* InInstance) { CustomizableObjectInstance = InInstance; }

	UFUNCTION(BlueprintPure, Category = "Custom")
	UCustomizableObjectInstance* GetCustomizableObjectInstance() const { return CustomizableObjectInstance; }
	UFUNCTION()
	void OnRep_DisplayName();

	FPDOnCharacterCustomInfoChangedNative& OnCharacterCustomInfoChangedNative() { return CharacterCustomInfoChangedNative; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;
	void HandleCharacterCustomInfoChanged();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	ETeamType TeamID;

	UPROPERTY(ReplicatedUsing = OnRep_DisplayName, BlueprintReadOnly, Category = "Team")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Custom")
	TObjectPtr<UCustomizableObjectInstance> CustomizableObjectInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<UPDAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<UPDAttributeSetBase> AttributeSetBase;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> GE_DeathClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> GE_ReviveClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inven")
	TObjectPtr<UPDInventoryComponent> InventoryComponent;

	FPDOnCharacterCustomInfoChangedNative CharacterCustomInfoChangedNative;
};
