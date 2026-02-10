#include "Controller/PDPlayerController.h"
#include "Blueprint/UserWidget.h"

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
	}
}