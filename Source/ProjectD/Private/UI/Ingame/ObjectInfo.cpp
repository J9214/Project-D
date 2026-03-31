// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Ingame/ObjectInfo.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UObjectInfo::SetInterfaceColor(FLinearColor NewColor)
{
	if (InfoImage)
	{
		InfoImage->SetColorAndOpacity(NewColor);
	}

	if (FillImage)
	{
		FillImage->SetColorAndOpacity(NewColor);
	}
}

void UObjectInfo::SetFillAmount(float Percent)
{
	if (FillImage)
	{
		float ClampedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
		FillImage->SetRenderScale(FVector2D(1.0f, ClampedPercent));
	}
}

void UObjectInfo::UpdateDistanceUI(int32 InDistance)
{
	if (!DistanceText)
	{
		return;
	}

	if (Distance == InDistance)
	{
		return;
	}

	if (InDistance <= 1.0f)
	{
		if (bCurrentVisible)
		{
			SetVisibility(ESlateVisibility::Hidden);
			bCurrentVisible = false;
		}
	}
	else
	{
		if (!bCurrentVisible)
		{
			SetVisibility(ESlateVisibility::HitTestInvisible);
			bCurrentVisible = true;
		}

		DistanceText->SetText(FText::Format(FText::FromString(TEXT("{0}")), InDistance));
	}

	Distance = InDistance;
}
