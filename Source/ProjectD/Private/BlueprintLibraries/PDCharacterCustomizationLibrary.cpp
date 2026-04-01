#include "BlueprintLibraries/PDCharacterCustomizationLibrary.h"

#include "MuCO/CustomizableObjectInstance.h"

namespace
{
FString DescribeCharacterCustomInfo(const FPDCharacterCustomInfo& CharacterInfo)
{
	FString Summary = FString::Printf(
		TEXT("CharacterId=%d Enum=%d Float=%d Color=%d Bool=%d"),
		CharacterInfo.CharacterId,
		CharacterInfo.EnumParameters.Num(),
		CharacterInfo.FloatParameters.Num(),
		CharacterInfo.ColorParameters.Num(),
		CharacterInfo.BoolParameters.Num());

	for (int32 Index = 0; Index < CharacterInfo.EnumParameters.Num(); ++Index)
	{
		const FPDMutableEnumParameter& Param = CharacterInfo.EnumParameters[Index];
		Summary += FString::Printf(TEXT(" | Enum[%d]={Name='%s',Option='%s',Range=%d}"), Index, *Param.ParameterName, *Param.SelectedOptionName, Param.RangeIndex);
	}

	for (int32 Index = 0; Index < CharacterInfo.FloatParameters.Num(); ++Index)
	{
		const FPDMutableFloatParameter& Param = CharacterInfo.FloatParameters[Index];
		Summary += FString::Printf(TEXT(" | Float[%d]={Name='%s',Value=%.3f,Range=%d}"), Index, *Param.ParameterName, Param.Value, Param.RangeIndex);
	}

	for (int32 Index = 0; Index < CharacterInfo.ColorParameters.Num(); ++Index)
	{
		const FPDMutableColorParameter& Param = CharacterInfo.ColorParameters[Index];
		Summary += FString::Printf(TEXT(" | Color[%d]={Name='%s',Value=%s}"), Index, *Param.ParameterName, *Param.Value.ToString());
	}

	for (int32 Index = 0; Index < CharacterInfo.BoolParameters.Num(); ++Index)
	{
		const FPDMutableBoolParameter& Param = CharacterInfo.BoolParameters[Index];
		Summary += FString::Printf(TEXT(" | Bool[%d]={Name='%s',Value=%d}"), Index, *Param.ParameterName, Param.Value ? 1 : 0);
	}

	return Summary;
}
}

bool UPDCharacterCustomizationLibrary::ApplyCharacterCustomInfoToMutableInstance(
	UCustomizableObjectInstance* CustomizableObjectInstance,
	const FPDCharacterCustomInfo& CharacterCustomInfo,
	bool bUpdateSkeletalMesh,
	bool bIgnoreCloseDist,
	bool bForceHighPriority)
{
	if (!IsValid(CustomizableObjectInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CharacterCustomization] CustomizableObjectInstance is null."));
		return false;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[CharacterCustomizationFlow][MutableLibrary::ApplyCharacterCustomInfoToMutableInstance] COI=%s Update=%d IgnoreClose=%d ForceHigh=%d %s"),
		*GetNameSafe(CustomizableObjectInstance),
		bUpdateSkeletalMesh ? 1 : 0,
		bIgnoreCloseDist ? 1 : 0,
		bForceHighPriority ? 1 : 0,
		*DescribeCharacterCustomInfo(CharacterCustomInfo));

	for (const FPDMutableEnumParameter& EnumParameter : CharacterCustomInfo.EnumParameters)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[CharacterCustomizationFlow][MutableLibrary::ApplyEnum] COI=%s Name='%s' Option='%s' Range=%d"),
			*GetNameSafe(CustomizableObjectInstance),
			*EnumParameter.ParameterName,
			*EnumParameter.SelectedOptionName,
			EnumParameter.RangeIndex);
		CustomizableObjectInstance->SetIntParameterSelectedOption(
			EnumParameter.ParameterName,
			EnumParameter.SelectedOptionName,
			EnumParameter.RangeIndex);
	}

	for (const FPDMutableFloatParameter& FloatParameter : CharacterCustomInfo.FloatParameters)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[CharacterCustomizationFlow][MutableLibrary::ApplyFloat] COI=%s Name='%s' Value=%.3f Range=%d"),
			*GetNameSafe(CustomizableObjectInstance),
			*FloatParameter.ParameterName,
			FloatParameter.Value,
			FloatParameter.RangeIndex);
		CustomizableObjectInstance->SetFloatParameterSelectedOption(
			FloatParameter.ParameterName,
			FloatParameter.Value,
			FloatParameter.RangeIndex);
	}

	for (const FPDMutableColorParameter& ColorParameter : CharacterCustomInfo.ColorParameters)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[CharacterCustomizationFlow][MutableLibrary::ApplyColor] COI=%s Name='%s' Value=%s"),
			*GetNameSafe(CustomizableObjectInstance),
			*ColorParameter.ParameterName,
			*ColorParameter.Value.ToString());
		CustomizableObjectInstance->SetColorParameterSelectedOption(
			ColorParameter.ParameterName,
			ColorParameter.Value);
	}

	for (const FPDMutableBoolParameter& BoolParameter : CharacterCustomInfo.BoolParameters)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[CharacterCustomizationFlow][MutableLibrary::ApplyBool] COI=%s Name='%s' Value=%d"),
			*GetNameSafe(CustomizableObjectInstance),
			*BoolParameter.ParameterName,
			BoolParameter.Value ? 1 : 0);
		CustomizableObjectInstance->SetBoolParameterSelectedOption(
			BoolParameter.ParameterName,
			BoolParameter.Value);
	}

	if (bUpdateSkeletalMesh)
	{
		CustomizableObjectInstance->UpdateSkeletalMeshAsync(bIgnoreCloseDist, bForceHighPriority);
	}

	return true;
}
