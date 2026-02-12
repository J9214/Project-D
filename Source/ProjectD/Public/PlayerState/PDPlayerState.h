#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Interface/PDTeamInterface.h"
#include "PDPlayerState.generated.h"

class UPDAbilitySystemComponent;
class UPDAttributeSetBase;
class UGameplayEffect;

UCLASS()
class PROJECTD_API APDPlayerState : public APlayerState, public IAbilitySystemInterface, public IPDTeamInterface
{
	GENERATED_BODY()
	
public:
	APDPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual ETeamType GetTeamID() const override { return TeamID; }
	
	UFUNCTION(BlueprintPure)
	UPDAbilitySystemComponent* GetPDAbilitySystemComponent() const { return AbilitySystemComponent; }
	UPDAttributeSetBase* GetPDAttributeSetBase() const { return AttributeSetBase; }

	void InitAbilityActorInfo(AActor* AvatarActor);

	void SetDeadState();

	void SetReviveState();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeamType TeamID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<UPDAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<UPDAttributeSetBase> AttributeSetBase;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> GE_DeathClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> GE_ReviveClass;
};