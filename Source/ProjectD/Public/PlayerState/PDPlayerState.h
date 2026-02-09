#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "PDPlayerState.generated.h"

class UPDAbilitySystemComponent;
class UPDAttributeSetBase;

UCLASS()
class PROJECTD_API APDPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	APDPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UPDAbilitySystemComponent* GetPDAbilitySystemComponent() const { return AbilitySystemComponent; }
	UPDAttributeSetBase* GetPDAttributeSetBase() const { return AttributeSetBase; }

	void InitAbilityActorInfo(AActor* AvatarActor);

	UPROPERTY(ReplicatedUsing = OnRep_TeamId, BlueprintReadOnly, Category = "Team")
	int32 TeamId = -1;

	UFUNCTION()
	void OnRep_TeamId();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<UPDAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<UPDAttributeSetBase> AttributeSetBase;
};
