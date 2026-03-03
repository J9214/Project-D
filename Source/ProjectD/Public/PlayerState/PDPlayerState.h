#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameInstance/PDCharacterCustomInfo.h"
#include "Interface/PDTeamInterface.h"
#include "PDPlayerState.generated.h"

class UPDAbilitySystemComponent;
class UPDAttributeSetBase;
class UGameplayEffect;
class UPDInventoryComponent;

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

	UPROPERTY(ReplicatedUsing = OnRep_CharacterCustomInfo, BlueprintReadOnly, Category = "Custom")
	FPDCharacterCustomInfo CharacterCustomInfo;

	UFUNCTION(BlueprintCallable)
	void SetDisplayName(const FString& NewDisplayName) { DisplayName = NewDisplayName; }

	UFUNCTION()
	void OnRep_CharacterCustomInfo();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	ETeamType TeamID;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	FString DisplayName;

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
};