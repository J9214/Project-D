#pragma once

#include "CoreMinimal.h"
#include "PDInteractableObject.generated.h"

UINTERFACE(MinimalAPI)
class UPDInteractableObject : public UInterface
{
	GENERATED_BODY()
};

class PROJECTD_API IPDInteractableObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gimmick|Interact")
	void OnInteract(AActor* Interactor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gimmick|Interact")
	void OnEndInteract(AActor* Interactor);
};
