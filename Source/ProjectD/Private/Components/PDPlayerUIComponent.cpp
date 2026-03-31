// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PDPlayerUIComponent.h"
#include "Pawn/PDPawnBase.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/PDPlayerState.h"
#include "UI/Ingame/IngmaeHPBar.h"

// Sets default values for this component's properties
UPDPlayerUIComponent::UPDPlayerUIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;

	Accum = 0.f;
}

// Called every frame
void UPDPlayerUIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	if (!HeadHPWidgetComp)
	{
		return;
	}

	if (!IsNetMode(NM_Standalone) && !GetWorld())
	{
		return;
	}

	if (!CameraManager.IsValid())
	{
		CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
		return;
	}

	const FVector CamLoc = CameraManager->GetCameraLocation();
	const FVector WidgetLoc = HeadHPWidgetComp->GetComponentLocation();
	const FRotator LookRot = (CamLoc - WidgetLoc).Rotation();

	HeadHPWidgetComp->SetWorldRotation(FRotator(0.f, LookRot.Yaw, 0.f));
}

void UPDPlayerUIComponent::InitComponents(APDPawnBase* InOwnerCharacter, UWidgetComponent* InHeadHPWidgetComp, UPDAttributeSetBase* InCharacterAttributeSet)
{
	OwnerCharacter = InOwnerCharacter;
	HeadHPWidgetComp = InHeadHPWidgetComp;
	CharacterAttributeSet = InCharacterAttributeSet;
}

void UPDPlayerUIComponent::SetupHeadHPWidget()
{
	if (!IsValid(HeadHPWidgetComp))
	{
		return;
	}

	if (!HeadHPWidgetClass)
	{
		return;
	}

	if (HeadHPWidgetComp->GetWidgetClass() != HeadHPWidgetClass)
	{
		HeadHPWidgetComp->SetWidgetClass(HeadHPWidgetClass);
	}

	HeadHPWidgetComp->InitWidget();

	HeadHPWidget = Cast<UIngmaeHPBar>(HeadHPWidgetComp->GetUserWidgetObject());

	if (!IsValid(HeadHPWidget))
	{
		return;
	}

}

void UPDPlayerUIComponent::OnHealthChanged(float OldValue, float NewValue)
{
	if (!IsValid(HeadHPWidget))
	{
		return;
	}

	HeadHPWidget->HandleHealthChanged(OldValue, NewValue);
}

void UPDPlayerUIComponent::SetPlayerNickName(const FString& InNickName, ETeamType LocalTeamID, ETeamType TargetTeamID)
{
	if (!IsValid(HeadHPWidget))
	{
		return;
	}
	HeadHPWidget->Init(InNickName);
	HeadHPWidget->SetTeamTextColor(LocalTeamID, TargetTeamID);
	HeadHPWidget->SetTeamColor(LocalTeamID == TargetTeamID);
}

