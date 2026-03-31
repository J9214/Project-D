#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameInstance/PDCharacterCustomInfo.h"
#include "MuCO/CustomizableObjectInstance.h"
#include "PDCharacterCustomizationLibrary.generated.h"

UCLASS()
class PROJECTD_API UPDCharacterCustomizationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Custom")
	static bool ApplyCharacterCustomInfoToMutableInstance(
		UCustomizableObjectInstance* CustomizableObjectInstance,
		const FPDCharacterCustomInfo& CharacterCustomInfo,
		bool bUpdateSkeletalMesh = true,
		bool bIgnoreCloseDist = true,
		bool bForceHighPriority = true);
};
