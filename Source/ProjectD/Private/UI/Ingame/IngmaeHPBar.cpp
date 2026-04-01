// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/IngmaeHPBar.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "UI/PDTeamColorFunctionLibrary.h"
#include <PlayerState/PDPlayerState.h>
#include "AttributeSet/PDAttributeSetBase.h"

bool UIngmaeHPBar::CacheBarMaterials(const TCHAR* Context)
{
    if (!BarFill)
    {
        LogWidgetState(ELogVerbosity::Error, Context, TEXT("BarFill binding is null. Check the widget blueprint hierarchy and variable name."));
        return false;
    }

    if (!BarGlow)
    {
        LogWidgetState(ELogVerbosity::Error, Context, TEXT("BarGlow binding is null. Check the widget blueprint hierarchy and variable name."));
        return false;
    }

    if (!CachedBarFillMID)
    {
        CachedBarFillMID = BarFill->GetDynamicMaterial();
        if (!CachedBarFillMID)
        {
            LogWidgetState(ELogVerbosity::Error, Context, TEXT("BarFill dynamic material is null. Check the image brush material assignment."));
            return false;
        }
    }

    if (!CachedBarGlowMID)
    {
        CachedBarGlowMID = BarGlow->GetDynamicMaterial();
        if (!CachedBarGlowMID)
        {
            LogWidgetState(ELogVerbosity::Error, Context, TEXT("BarGlow dynamic material is null. Check the image brush material assignment."));
            return false;
        }
    }

    if (!NickName)
    {
        LogWidgetState(ELogVerbosity::Warning, Context, TEXT("NickName binding is null. Display name updates will be skipped."));
    }

    return true;
}

bool UIngmaeHPBar::CacheBarFillMaterial(const TCHAR* Context)
{
    if (!BarFill)
    {
        LogWidgetState(ELogVerbosity::Error, Context, TEXT("BarFill binding is null. Check the widget blueprint hierarchy and variable name."));
        return false;
    }

    if (!CachedBarFillMID)
    {
        CachedBarFillMID = BarFill->GetDynamicMaterial();
        if (!CachedBarFillMID)
        {
            LogWidgetState(ELogVerbosity::Error, Context, TEXT("BarFill dynamic material is null. Check the image brush material assignment."));
            return false;
        }
    }

    return true;
}

FString UIngmaeHPBar::BuildWidgetTreeSummary() const
{
    if (!WidgetTree)
    {
        return TEXT("<NoWidgetTree>");
    }

    TArray<UWidget*> AllWidgets;
    WidgetTree->GetAllWidgets(AllWidgets);

    TArray<FString> WidgetDescriptions;
    WidgetDescriptions.Reserve(AllWidgets.Num());

    for (const UWidget* Widget : AllWidgets)
    {
        WidgetDescriptions.Add(FString::Printf(TEXT("%s(%s)"), *GetNameSafe(Widget), *GetNameSafe(Widget ? Widget->GetClass() : nullptr)));
    }

    return WidgetDescriptions.IsEmpty() ? TEXT("<EmptyWidgetTree>") : FString::Join(WidgetDescriptions, TEXT(", "));
}

void UIngmaeHPBar::LogWidgetState(ELogVerbosity::Type Verbosity, const TCHAR* Context, const TCHAR* Detail) const
{
    const FString WidgetPath = GetPathNameSafe(this);
    const FString ClassPath = GetPathNameSafe(GetClass());
    const FString OwningPlayerPath = GetPathNameSafe(GetOwningPlayer());
    const FString BarFillName = GetNameSafe(BarFill);
    const FString BarGlowName = GetNameSafe(BarGlow);
    const FString NickNameName = GetNameSafe(NickName);
    const FString DamagedName = GetNameSafe(Damaged);
    const FString TeamDamagedName = GetNameSafe(TeamDamaged);
    const FString WidgetTreeSummary = BuildWidgetTreeSummary();

    switch (Verbosity)
    {
    case ELogVerbosity::Error:
        UE_LOG(LogTemp, Error, TEXT("[IngmaeHPBar][%s] %s Widget=%s Class=%s OwningPlayer=%s BarFill=%s BarGlow=%s NickName=%s Damaged=%s TeamDamaged=%s WidgetTree=%s"),
            Context, Detail, *WidgetPath, *ClassPath, *OwningPlayerPath, *BarFillName, *BarGlowName, *NickNameName, *DamagedName, *TeamDamagedName, *WidgetTreeSummary);
        break;
    case ELogVerbosity::Warning:
        UE_LOG(LogTemp, Warning, TEXT("[IngmaeHPBar][%s] %s Widget=%s Class=%s OwningPlayer=%s BarFill=%s BarGlow=%s NickName=%s Damaged=%s TeamDamaged=%s WidgetTree=%s"),
            Context, Detail, *WidgetPath, *ClassPath, *OwningPlayerPath, *BarFillName, *BarGlowName, *NickNameName, *DamagedName, *TeamDamagedName, *WidgetTreeSummary);
        break;
    default:
        UE_LOG(LogTemp, Log, TEXT("[IngmaeHPBar][%s] %s Widget=%s Class=%s OwningPlayer=%s BarFill=%s BarGlow=%s NickName=%s Damaged=%s TeamDamaged=%s WidgetTree=%s"),
            Context, Detail, *WidgetPath, *ClassPath, *OwningPlayerPath, *BarFillName, *BarGlowName, *NickNameName, *DamagedName, *TeamDamagedName, *WidgetTreeSummary);
        break;
    }
}

void UIngmaeHPBar::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    IsInit = false;

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        LogWidgetState(ELogVerbosity::Warning, TEXT("NativeOnInitialized"), TEXT("OwningPlayer is null."));
        return;
    }

    APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>();
    if (!PS)
    {
        LogWidgetState(ELogVerbosity::Warning, TEXT("NativeOnInitialized"), TEXT("Owning player state is null."));
        return;
    }

    Init(PS->GetDisplayName());
}

void UIngmaeHPBar::Init(const FString& DisplayName)
{
    SetDisplayName(DisplayName);

    if (!CacheBarMaterials(TEXT("Init")))
    {
        return;
    }

    if (IsTeam)
    {
        if (Damaged)
        {
            StopAnimation(Damaged);
        }
    }
    else
    {
        if (TeamDamaged)
        {
            StopAnimation(TeamDamaged);
        }
    }

    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthCurrent"), 1);
    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthUpdate"), 1);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Current"), 1);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Updated"), 1);

    IsInit = true;
}

void UIngmaeHPBar::SetDisplayName(const FString& DisplayName)
{
    if (NickName)
    {
        NickName->SetText(FText::FromString(DisplayName));
    }
}

void UIngmaeHPBar::HandleHealthChanged(float OldValue, float NewValue)
{
    if (MaxHPValue <= 0.0f)
    {
        return;
    }

    NowHPValue = NewValue;

    if (!CacheBarMaterials(TEXT("HandleHealthChanged")))
    {
        return;
    }

    const float HealthRatio = FMath::Clamp(NowHPValue / MaxHPValue, 0.0f, 1.0f);
    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthCurrent"), HealthRatio);
    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthUpdate"), HealthRatio);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Current"), HealthRatio);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Updated"), HealthRatio);

    if (OldValue > NewValue)
    {
        if (IsTeam)
        {
            if (TeamDamaged)
            {
                PlayAnimation(TeamDamaged, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
            }
        }
        else
        {
            if (Damaged)
            {
                PlayAnimation(Damaged, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
            }
        }
    }

}

void UIngmaeHPBar::SetMaxHealth(float NewMaxHealth)
{
    MaxHPValue = FMath::Max(NewMaxHealth, 1.0f);

    if (!CacheBarMaterials(TEXT("SetMaxHealth")))
    {
        return;
    }

    const float HealthRatio = FMath::Clamp(NowHPValue / MaxHPValue, 0.0f, 1.0f);
    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthCurrent"), HealthRatio);
    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthUpdate"), HealthRatio);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Current"), HealthRatio);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Updated"), HealthRatio);
}

void UIngmaeHPBar::SetPlayerColor()
{
    if (!CacheBarFillMaterial(TEXT("SetPlayerColor")))
    {
        return;
    }

    CachedBarFillMID->SetScalarParameterValue(TEXT("PlayerCheck"), 1);
}

//0 팀
//1 적
void UIngmaeHPBar::SetTeamColor(bool TeamType)
{
    if (!CacheBarFillMaterial(TEXT("SetTeamColor")))
    {
        return;
    }
    IsTeam = TeamType;

    CachedBarFillMID->SetScalarParameterValue(TEXT("PlayerCheck"), 0);
    CachedBarFillMID->SetScalarParameterValue(TEXT("TeamCheck"), TeamType);

	if (IsTeam)
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		SetVisibility(ESlateVisibility::Visible);
	}
}

void UIngmaeHPBar::SetTeamTextColor(ETeamType LocalTeamID, ETeamType TargetTeamID)
{
	if (!NickName)
	{
		return;
	}

	NickName->SetColorAndOpacity(UPDTeamColorFunctionLibrary::GetRelativeTeamSlateColor(LocalTeamID, TargetTeamID));
}

