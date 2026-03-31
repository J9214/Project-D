// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/PDTeamHPInfo.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/PDTeamColorFunctionLibrary.h"
#include <PlayerState/PDPlayerState.h>


void UPDTeamHPInfo::NativeOnInitialized()
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

void UPDTeamHPInfo::Init(const FString& DisplayName)
{
    if (NickName)
    {
        NickName->SetText(FText::FromString(DisplayName));
    }

    CachedBarFillMID = BarFill->GetDynamicMaterial();
    CachedBarGlowMID = BarGlow->GetDynamicMaterial();

    if (!CachedBarFillMID || !CachedBarGlowMID)
    {
        return;
    }

    StopAnimation(Damaged);

    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthCurrent"), 1);
    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthUpdate"), 1);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Current"), 1);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Updated"), 1);

    IsInit = true;
}

float UPDTeamHPInfo::HandleHealthChanged(float OldValue, float NewValue)
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

    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthCurrent"), SetOldValue);
    CachedBarFillMID->SetScalarParameterValue(TEXT("HealthUpdate"), SetNewValue);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Current"), SetOldValue);
    CachedBarGlowMID->SetScalarParameterValue(TEXT("Health_Updated"), SetNewValue);

    if (OldValue > NewValue)
    {
        PlayAnimation(Damaged, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
    }

    return SetNewValue;
}

void UPDTeamHPInfo::SetPlayerColor()
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
void UPDTeamHPInfo::SetTeamColor(bool TeamType)
{
    if (!CachedBarFillMID)
    {
        CachedBarFillMID = BarFill->GetDynamicMaterial();
    }

    if (!CachedBarFillMID)
    {
        return;
    }

	CachedBarFillMID->SetScalarParameterValue(TEXT("PlayerCheck"), 0);
	CachedBarFillMID->SetScalarParameterValue(TEXT("TeamCheck"), TeamType);
}

void UPDTeamHPInfo::SetTeamTextColor(ETeamType LocalTeamID, ETeamType TargetTeamID)
{
	if (!NickName)
	{
		return;
	}

	NickName->SetColorAndOpacity(UPDTeamColorFunctionLibrary::GetRelativeTeamSlateColor(LocalTeamID, TargetTeamID));
}

