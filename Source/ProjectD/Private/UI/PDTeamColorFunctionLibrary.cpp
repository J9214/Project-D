#include "UI/PDTeamColorFunctionLibrary.h"

bool UPDTeamColorFunctionLibrary::IsPlayableTeam(ETeamType TeamID)
{
	return TeamID == ETeamType::TeamOne
		|| TeamID == ETeamType::TeamTwo
		|| TeamID == ETeamType::TeamThree;
}

FLinearColor UPDTeamColorFunctionLibrary::GetRelativeTeamColor(ETeamType LocalTeamID, ETeamType TargetTeamID)
{
	if (!IsPlayableTeam(LocalTeamID) || !IsPlayableTeam(TargetTeamID))
	{
		return FLinearColor::White;
	}

	if (LocalTeamID == TargetTeamID)
	{
		return FLinearColor::Green;
	}

	switch (LocalTeamID)
	{
	case ETeamType::TeamOne:
		return (TargetTeamID == ETeamType::TeamTwo) ? FLinearColor::Red : FLinearColor::Yellow;

	case ETeamType::TeamTwo:
		return (TargetTeamID == ETeamType::TeamOne) ? FLinearColor::Red : FLinearColor::Yellow;

	case ETeamType::TeamThree:
		return (TargetTeamID == ETeamType::TeamOne) ? FLinearColor::Red : FLinearColor::Yellow;

	default:
		return FLinearColor::White;
	}
}

FSlateColor UPDTeamColorFunctionLibrary::GetRelativeTeamSlateColor(ETeamType LocalTeamID, ETeamType TargetTeamID)
{
	return FSlateColor(GetRelativeTeamColor(LocalTeamID, TargetTeamID));
}

ETeamType UPDTeamColorFunctionLibrary::GetTeamTypeFromIndex(int32 TeamIndex)
{
	switch (TeamIndex)
	{
	case 0:
		return ETeamType::TeamOne;

	case 1:
		return ETeamType::TeamTwo;

	case 2:
		return ETeamType::TeamThree;

	default:
		return ETeamType::None;
	}
}

FLinearColor UPDTeamColorFunctionLibrary::GetRelativeTeamColorByIndex(ETeamType LocalTeamID, int32 TargetTeamIndex)
{
	return GetRelativeTeamColor(LocalTeamID, GetTeamTypeFromIndex(TargetTeamIndex));
}

FSlateColor UPDTeamColorFunctionLibrary::GetRelativeTeamSlateColorByIndex(ETeamType LocalTeamID, int32 TargetTeamIndex)
{
	return FSlateColor(GetRelativeTeamColorByIndex(LocalTeamID, TargetTeamIndex));
}
