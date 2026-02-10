#include "Controller/PDPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "GameState/PDGameStateBase.h"

void APDPlayerController::ClientShowGameOver_Implementation(int32 WinningTeam)
{
	SetInputMode(FInputModeUIOnly());
}

void APDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(IsLocalPlayerController())
	{
		if (PlayerHUDClass) {
			UUserWidget* PlayerHUD = CreateWidget<UUserWidget>(this, PlayerHUDClass);
			if (PlayerHUD)
			{
				PlayerHUD->AddToViewport();
			}
		}

		if (ResultWidgetClass)
		{
			ResultWidget = CreateWidget<UUserWidget>(this, ResultWidgetClass);
			if (ResultWidget)
			{
				ResultWidget->AddToViewport(10);
			}
		}
	}
}