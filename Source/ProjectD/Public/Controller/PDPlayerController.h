#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PDPlayerController.generated.h"

UCLASS()
class PROJECTD_API APDPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDClass;
protected:
	virtual void BeginPlay() override;

};
