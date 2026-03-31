// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/PDTeamInterface.h"
#include "PDPlayerUIComponent.generated.h"

class APDPawnBase;
class UWidgetComponent;
class UPDAttributeSetBase;
class UIngmaeHPBar;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTD_API UPDPlayerUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPDPlayerUIComponent();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitComponents(APDPawnBase* InOwnerCharacter, UWidgetComponent* InHeadHPWidgetComp, UPDAttributeSetBase* InCharacterAttributeSet);

	void SetupHeadHPWidget();

	UFUNCTION()
	void OnHealthChanged(float OldValue, float NewValue);

	UFUNCTION()
	void OnMaxHealthChanged(float OldValue, float NewValue);

	void SetPlayerNickName(const FString& InNickName, ETeamType LocalTeamID, ETeamType TargetTeamID);

protected:

	UPROPERTY()
	TWeakObjectPtr<APDPawnBase> OwnerCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HeadHPWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UIngmaeHPBar> HeadHPWidgetClass;

	UPROPERTY()
	TObjectPtr<UIngmaeHPBar> HeadHPWidget;

	UPROPERTY()
	TWeakObjectPtr<UPDAttributeSetBase> CharacterAttributeSet;

	UPROPERTY()
	TWeakObjectPtr<APlayerCameraManager> CameraManager;

	float Accum;
};
