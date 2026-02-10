#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "PDEnhancedInputComponent.generated.h"

UCLASS()
class PROJECTD_API UPDEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	template<class UserObject, typename CallbackFunc>
	void BindAbilityInputAction(
		const UDataAsset_InputConfig* InInputConfig,
		UserObject* ContextObject,
		CallbackFunc InputPressedFunc,
		CallbackFunc InputReleasedFunc
	);
};

template<class UserObject, typename CallbackFunc>
inline void UPDEnhancedInputComponent::BindAbilityInputAction(
	const UDataAsset_InputConfig* InInputConfig,
	UserObject* ContextObject,
	CallbackFunc InputPressedFunc,
	CallbackFunc InputReleasedFunc
)
{
	checkf(InInputConfig, TEXT("Input config data asset is null"));

	for (const FPDInputActionConfig& AbilityInputActionConfig : InInputConfig->AbilityInputActions)
	{
		if (!AbilityInputActionConfig.IsValid())
		{
			continue;
		}

		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Started, ContextObject, InputPressedFunc, AbilityInputActionConfig.InputTag);
		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Completed, ContextObject, InputReleasedFunc, AbilityInputActionConfig.InputTag);
		BindAction(AbilityInputActionConfig.InputAction, ETriggerEvent::Canceled, ContextObject, InputReleasedFunc, AbilityInputActionConfig.InputTag);
	}
}
