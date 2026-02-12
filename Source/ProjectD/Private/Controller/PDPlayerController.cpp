#include "Controller/PDPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "GameState/PDGameStateBase.h"

void APDPlayerController::ShowGameOver()
{
	if (ResultWidget)
	{
		if (APDGameStateBase* GS = GetWorld()->GetGameState<APDGameStateBase>())
		{
			// 여기서 위젯에 승리 팀 정보 전달
		}
		ResultWidget->SetVisibility(ESlateVisibility::Visible);
		SetInputMode(FInputModeUIOnly());
	}
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