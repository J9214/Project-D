// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/IngmaeHPBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/PDTeamColorFunctionLibrary.h"
#include <PlayerState/PDPlayerState.h>
#include "AttributeSet/PDAttributeSetBase.h"

void UIngmaeHPBar::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    APDPlayerState* PS = PC->GetPlayerState<APDPlayerState>();
    if (!PS)
    {
        return;
    }

    Init(PS->GetDisplayName());
    IsInit = true;
}

void UIngmaeHPBar::Init(const FString& DisplayName)
{
    SetDisplayName(DisplayName);

    CachedBarFillMID = BarFill->GetDynamicMaterial();
    CachedBarGlowMID = BarGlow->GetDynamicMaterial();

    if (!CachedBarFillMID || !CachedBarGlowMID)
    {
        return;
    }

    if (IsTeam)
    {
        StopAnimation(Damaged);
    }
    else
    {
        StopAnimation(TeamDamaged);
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

float UIngmaeHPBar::HandleHealthChanged(float OldValue, float NewValue)
{
    NowHPValue = NewValue;

    if (MaxHPValue == 0)
    {
        return 0.f;
    }

    float SetOldValue = OldValue / MaxHPValue;
    float SetNewValue = NewValue / MaxHPValue;

    if (!CachedBarFillMID || !CachedBarGlowMID)
    {
        return 0.f;
    }

    StopAnimation(Damaged);

    if (IsTeam)
    {
        StopAnimation(Damaged);
    }
    else
    {
        StopAnimation(TeamDamaged);
    }

    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthCurrent"), SetOldValue);
    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthUpdate"), SetNewValue);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Current"), SetOldValue);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Updated"), SetNewValue);

    if (OldValue > NewValue)
    {
        if (IsTeam)
        {
            PlayAnimation(TeamDamaged, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
        }
        else
        {
            PlayAnimation(Damaged, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
        }
    }

    return SetNewValue;
}

void UIngmaeHPBar::SetMaxHealth(float NewMaxHealth)
{
    MaxHPValue = FMath::Max(NewMaxHealth, 1.0f);

    if (!CachedBarFillMID || !CachedBarGlowMID)
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
    if (!CachedBarFillMID)
    {
        CachedBarFillMID = BarFill->GetDynamicMaterial();
    }

    if (!CachedBarFillMID)
    {
        return;
    }

    CachedBarFillMID->SetScalarParameterValue(TEXT("PlayerCheck"), 1);
}

//0 팀
//1 적
void UIngmaeHPBar::SetTeamColor(bool TeamType)
{
    if (!CachedBarFillMID)
    {
        CachedBarFillMID = BarFill->GetDynamicMaterial();
    }

    if (!CachedBarFillMID)
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

