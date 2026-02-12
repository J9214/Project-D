#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interface/PDTeamInterface.h"
#include "PDPlayerController.generated.h"

UCLASS()
class PROJECTD_API APDPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void ShowGameOver();
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ResultWidgetClass;
	UPROPERTY()
	UUserWidget* ResultWidget;
};
