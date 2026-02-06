#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Interface/PDTeamInterface.h"
#include "PDPlayerState.generated.h"

class UPDAbilitySystemComponent;
class UPDAttributeSetBase;

UCLASS()
class PROJECTD_API APDPlayerState : public APlayerState, public IAbilitySystemInterface, public IPDTeamInterface
{
	GENERATED_BODY()
	
public:
	APDPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual ETeamType GetTeamID() const override { return TeamID; }

	UPDAbilitySystemComponent* GetPDAbilitySystemComponent() const { return AbilitySystemComponent; }
	UPDAttributeSetBase* GetPDAttributeSetBase() const { return AttributeSetBase; }

	void InitAbilityActorInfo(AActor* AvatarActor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeamType TeamID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<UPDAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<UPDAttributeSetBase> AttributeSetBase;
};
