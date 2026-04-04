// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PDPlayerUIComponent.h"
#include "Pawn/PDPawnBase.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
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
	if (CharacterAttributeSet.IsValid())
	{
		CharacterAttributeSet->OnHealthChanged.RemoveDynamic(this, &ThisClass::OnHealthChanged);
		CharacterAttributeSet->OnMaxHealthChanged.RemoveDynamic(this, &ThisClass::OnMaxHealthChanged);
	}

	OwnerCharacter = InOwnerCharacter;
	HeadHPWidgetComp = InHeadHPWidgetComp;
	CharacterAttributeSet = InCharacterAttributeSet;

	SetupHeadHPWidget();

	if (CharacterAttributeSet.IsValid())
	{
		CharacterAttributeSet->OnHealthChanged.AddUniqueDynamic(this, &ThisClass::OnHealthChanged);
		CharacterAttributeSet->OnMaxHealthChanged.AddUniqueDynamic(this, &ThisClass::OnMaxHealthChanged);

		OnMaxHealthChanged(CharacterAttributeSet->GetMaxHealth(), CharacterAttributeSet->GetMaxHealth());
		OnHealthChanged(CharacterAttributeSet->GetHealth(), CharacterAttributeSet->GetHealth());
	}
}

void UPDPlayerUIComponent::SetupHeadHPWidget()
{
	if (!IsValid(HeadHPWidgetComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("[HeadHPWidget] Widget component is null. Owner=%s"), *GetPathNameSafe(GetOwner()));
		return;
	}

	UClass* ConfiguredWidgetClass = HeadHPWidgetClass.Get();
	UClass* ComponentWidgetClass = HeadHPWidgetComp->GetWidgetClass();

	if (ConfiguredWidgetClass && ComponentWidgetClass != ConfiguredWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HeadHPWidget] Overriding widget class. Owner=%s Component=%s Previous=%s Configured=%s"),
			*GetPathNameSafe(GetOwner()),
			*GetPathNameSafe(HeadHPWidgetComp),
			*GetPathNameSafe(ComponentWidgetClass),
			*GetPathNameSafe(ConfiguredWidgetClass));
		HeadHPWidgetComp->SetWidgetClass(ConfiguredWidgetClass);
		ComponentWidgetClass = ConfiguredWidgetClass;
	}
	else if (!ConfiguredWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HeadHPWidget] HeadHPWidgetClass is not set. Falling back to component widget class. Owner=%s Component=%s WidgetClass=%s"),
			*GetPathNameSafe(GetOwner()),
			*GetPathNameSafe(HeadHPWidgetComp),
			*GetPathNameSafe(ComponentWidgetClass));
	}

	if (!ComponentWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[HeadHPWidget] Widget class is null before InitWidget. Owner=%s Component=%s"),
			*GetPathNameSafe(GetOwner()),
			*GetPathNameSafe(HeadHPWidgetComp));
		return;
	}

	HeadHPWidgetComp->InitWidget();
	UUserWidget* CreatedWidget = HeadHPWidgetComp->GetUserWidgetObject();
	if (!IsValid(CreatedWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("[HeadHPWidget] InitWidget did not create a user widget. Owner=%s Component=%s WidgetClass=%s"),
			*GetPathNameSafe(GetOwner()),
			*GetPathNameSafe(HeadHPWidgetComp),
			*GetPathNameSafe(ComponentWidgetClass));
		return;
	}

	HeadHPWidget = Cast<UIngmaeHPBar>(CreatedWidget);

	if (!IsValid(HeadHPWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("[HeadHPWidget] Created widget type mismatch. Owner=%s Component=%s ExpectedBase=%s RequestedClass=%s CreatedWidget=%s CreatedClass=%s"),
			*GetPathNameSafe(GetOwner()),
			*GetPathNameSafe(HeadHPWidgetComp),
			*GetPathNameSafe(UIngmaeHPBar::StaticClass()),
			*GetPathNameSafe(ComponentWidgetClass),
			*GetPathNameSafe(CreatedWidget),
			*GetPathNameSafe(CreatedWidget->GetClass()));
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

void UPDPlayerUIComponent::OnMaxHealthChanged(float OldValue, float NewValue)
{
	if (!IsValid(HeadHPWidget))
	{
		return;
	}

	HeadHPWidget->SetMaxHealth(NewValue);
}

void UPDPlayerUIComponent::SetPlayerNickName(const FString& InNickName, ETeamType LocalTeamID, ETeamType TargetTeamID)
{
	if (!IsValid(HeadHPWidget))
	{
		return;
	}
	HeadHPWidget->SetDisplayName(InNickName);
	HeadHPWidget->SetTeamTextColor(LocalTeamID, TargetTeamID);
	HeadHPWidget->SetTeamColor(LocalTeamID == TargetTeamID);
}

