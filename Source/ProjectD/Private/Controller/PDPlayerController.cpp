#include "Controller/PDPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "Weapon/PDWeaponBase.h"
#include "Pawn/PDPawnBase.h"
#include "GameState/PDGameStateBase.h"

void APDPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void APDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void APDPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	
	if (IsLocalController())
	{
		BindWeaponChangedDelegate();
	}
}

void APDPlayerController::OnPossess(APawn* InPawn) 
{
	Super::OnPossess(InPawn);

	if (IsLocalController())
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

void APDPlayerController::OnUnPossess()
{
	if (IsLocalController())
	{
		UnbindWeaponChangedDelegate();
	}
	
	Super::OnUnPossess();
}

void APDPlayerController::ShowAimCrosshair(TSubclassOf<UUserWidget> CrosshairClass)
{
	if (!IsLocalController())
	{
		return;
	}
	
	if (CurrentCrosshairWidget && CurrentCrosshairClass == CrosshairClass)
	{
		return;
	}

	if (CurrentCrosshairWidget)
	{
		CurrentCrosshairWidget->RemoveFromParent();
		CurrentCrosshairWidget = nullptr;
		CurrentCrosshairClass = nullptr;
	}

	if (!CrosshairClass)
	{
		return;
	}
	
	CurrentCrosshairWidget = CreateWidget<UUserWidget>(this, CrosshairClass);
	if (CurrentCrosshairWidget)
	{
		CurrentCrosshairClass = CrosshairClass;
		CurrentCrosshairWidget->AddToViewport(50);
	}
}

void APDPlayerController::HideAimCrosshair()
{
	if (!IsLocalController())
	{
		return;
	}
	
	if (CurrentCrosshairWidget)
	{
		CurrentCrosshairWidget->RemoveFromParent();
		CurrentCrosshairWidget = nullptr;
		CurrentCrosshairClass = nullptr;
	}
}

void APDPlayerController::OnAimStarted()
{
	bWantsToAim = true;
	ShowAimCrosshair(GetCurrentAimCrosshairClass());
}

void APDPlayerController::OnAimEnded()
{
	bWantsToAim = false;
	HideAimCrosshair();
}

TSubclassOf<UUserWidget> APDPlayerController::GetCurrentAimCrosshairClass() const
{
	const APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetPawn());
	if (!OwnerPawn)
	{
		return nullptr;
	}
	
	const UWeaponManageComponent* WeaponComp = OwnerPawn->GetWeaponManageComponent();
	if (!WeaponComp)
	{
		return nullptr;
	}
	
	const APDWeaponBase* Weapon = WeaponComp->GetEquippedWeapon();
	if (!Weapon || !Weapon->WeaponData)
	{
		return nullptr;
	}
	
	return Weapon->WeaponData->AimCrosshairWidgetClass;
}

void APDPlayerController::HandleWeaponChanged(APDWeaponBase* NewWeapon)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!bWantsToAim || !NewWeapon)
	{
		HideAimCrosshair();
		return;
	}

	if (UDataAsset_Weapon* NewWeaponDA = NewWeapon->WeaponData)
	{
		if (TSubclassOf<UUserWidget> Crosshair = NewWeaponDA->AimCrosshairWidgetClass)
		{
			ShowAimCrosshair(Crosshair);
		}
	}
}

void APDPlayerController::BindWeaponChangedDelegate()
{
	UnbindWeaponChangedDelegate();

	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetPawn());
	if (!OwnerPawn)
	{
		return;
	}

	UWeaponManageComponent* WeaponComp = OwnerPawn->GetWeaponManageComponent();
	if (!WeaponComp)
	{
		return;
	}

	WeaponComp->OnEquippedWeaponDataChanged.AddUObject(this, &APDPlayerController::HandleWeaponChanged);

	if (bWantsToAim)
	{
		ShowAimCrosshair(GetCurrentAimCrosshairClass());
	}
}

void APDPlayerController::UnbindWeaponChangedDelegate() const
{
	APDPawnBase* OwnerPawn = Cast<APDPawnBase>(GetPawn());
	if (!OwnerPawn)
	{
		return;
	}

	UWeaponManageComponent* WeaponComp = OwnerPawn->GetWeaponManageComponent();
	if (!WeaponComp)
	{
		return;
	}

	WeaponComp->OnEquippedWeaponDataChanged.RemoveAll(this);
}

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
