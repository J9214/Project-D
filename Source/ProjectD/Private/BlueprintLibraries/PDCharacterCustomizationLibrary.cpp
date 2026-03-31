#include "BlueprintLibraries/PDCharacterCustomizationLibrary.h"

#include "MuCO/CustomizableObjectInstance.h"

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

	for (const FPDMutableEnumParameter& EnumParameter : CharacterCustomInfo.EnumParameters)
	{
		CustomizableObjectInstance->SetIntParameterSelectedOption(
			EnumParameter.ParameterName,
			EnumParameter.SelectedOptionName,
			EnumParameter.RangeIndex);
	}

	for (const FPDMutableFloatParameter& FloatParameter : CharacterCustomInfo.FloatParameters)
	{
		CustomizableObjectInstance->SetFloatParameterSelectedOption(
			FloatParameter.ParameterName,
			FloatParameter.Value,
			FloatParameter.RangeIndex);
	}

	for (const FPDMutableColorParameter& ColorParameter : CharacterCustomInfo.ColorParameters)
	{
		CustomizableObjectInstance->SetColorParameterSelectedOption(
			ColorParameter.ParameterName,
			ColorParameter.Value);
	}

	for (const FPDMutableBoolParameter& BoolParameter : CharacterCustomInfo.BoolParameters)
	{
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
