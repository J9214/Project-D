#include "Controller/PDPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "DataAssets/Weapon/DataAsset_Weapon.h"
#include "Weapon/PDWeaponBase.h"
#include "Pawn/PDPawnBase.h"
#include "GameState/PDGameStateBase.h"
#include "UI/HUD/IngameHUD.h"
#include "Components/Inventory/PDInventoryComponent.h"
#include "PlayerState/PDPlayerState.h"
#include "Components/Shop/PDShopComponent.h"
#include "Pawn/PDPawnBase.h"
#include "Components/PDPlayerUIComponent.h"
#include "GameMode/PDGameModeBase.h"

APDPlayerController::APDPlayerController()
{
	ShopComponent = CreateDefaultSubobject<UPDShopComponent>(TEXT("ShopComponent"));

}

void APDPlayerController::KillSelfCheat()
{
	if (HasAuthority())
	{
		if (APDGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APDGameModeBase>() : nullptr)
		{
			GameMode->PlayerDied(this);
		}
		else if (APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>())
		{
			PDPlayerState->SetDeadState();
		}

		return;
	}

	Server_KillSelfCheat();
}

void APDPlayerController::ReviveSelfCheat()
{
	if (HasAuthority())
	{
		if (APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>())
		{
			PDPlayerState->SetReviveState();
		}

		return;
	}

	Server_ReviveSelfCheat();
}

void APDPlayerController::Server_KillSelfCheat_Implementation()
{
	if (APDGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APDGameModeBase>() : nullptr)
	{
		GameMode->PlayerDied(this);
		return;
	}

	if (APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>())
	{
		PDPlayerState->SetDeadState();
	}
}

void APDPlayerController::Server_ReviveSelfCheat_Implementation()
{
	if (APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>())
	{
		PDPlayerState->SetReviveState();
	}
}

void APDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
#if UE_EDITOR

		if (PlayerHUDClass && !PlayerHUDWidget)
		{
			PlayerHUDWidget = CreateWidget<UIngameHUD>(this, PlayerHUDClass);

			if (PlayerHUDWidget)
			{
				PlayerHUDWidget->AddToViewport();
			}
		}
#else
		if (LoadingHUDClass && !LoadingHUD)
		{
			LoadingHUD = CreateWidget<UUserWidget>(this, LoadingHUDClass);
			if (LoadingHUD)
			{
				LoadingHUD->AddToViewport(100);
			}
		}

		StartReadyCheck();
	
#endif

	}
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

	WeaponComp->OnEquippedWeaponChanged.AddUObject(this, &APDPlayerController::HandleWeaponChanged);

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

	WeaponComp->OnEquippedWeaponChanged.RemoveAll(this);
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

void APDPlayerController::InitGoldDisplay(int InGold)
{
	if (!PlayerHUDWidget)
	{
		return;
	}

	PlayerHUDWidget->InitGold(InGold);
}

void APDPlayerController::InitItemDataDisplay(EItemType ItemType, int SlotIndex, const FName& NewItemID, int Count)
{
	if (!PlayerHUDWidget)
	{
		return;
	}

	PlayerHUDWidget->InitItem(ItemType, SlotIndex, NewItemID, Count);
}

void APDPlayerController::InitializeHUD()
{
	if (PlayerHUDClass && !PlayerHUDWidget)
	{
		PlayerHUDWidget = CreateWidget<UIngameHUD>(this, PlayerHUDClass);
		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->AddToViewport();
		}
	}
}

void APDPlayerController::UpdateCurrentAmmo(int32 CurrentAmmo)
{
	if (!PlayerHUDWidget)
	{
		return;
	}

	PlayerHUDWidget->UpdateCurrentAmmo(CurrentAmmo);
}

void APDPlayerController::StartReadyCheck()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("APDPlayerController::StartReadyCheck GetWorld false"));
		return;
	}

	GetWorldTimerManager().ClearTimer(ReadyCheckTimerHandle);

	GetWorldTimerManager().SetTimer(
		ReadyCheckTimerHandle,
		this,
		&APDPlayerController::TickReadyCheck,
		0.2f,
		true
	);
}

void APDPlayerController::TickReadyCheck()
{
	if (bLocalReadyReported)
	{
		UE_LOG(LogTemp, Warning, TEXT("APDPlayerController::TickReadyCheck IsReady"));
		return;
	}

	if (!AreAllPlayersReplicatedOnThisClient())
	{
		return;
	}

	bLocalReadyReported = true;
	ServerRPC_ReportClientReady();
	GetWorldTimerManager().ClearTimer(ReadyCheckTimerHandle);
}

bool APDPlayerController::AreAllPlayersReplicatedOnThisClient() const
{
	UWorld* World = GetWorld();
	AGameStateBase* GS = World ? World->GetGameState() : nullptr;
	if (!GS || ExpectedPlayerCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT(" APDPlayerController::AreAllPlayersReplicatedOnThisClient Player GS Loading Yet"));
		return false;
	}

	if (!GetPlayerState<APDPlayerState>() || !GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT(" APDPlayerController::AreAllPlayersReplicatedOnThisClient Player Pawn Loading Yet"));
		return false;
	}

	if (GS->PlayerArray.Num() < ExpectedPlayerCount)
	{
		UE_LOG(LogTemp, Warning, TEXT(" APDPlayerController::AreAllPlayersReplicatedOnThisClient All GS Loading Yet"));
		return false;
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS || !PS->GetPawn() || !PS->GetPawn()->HasActorBegunPlay())
		{
			UE_LOG(LogTemp, Warning, TEXT(" APDPlayerController::AreAllPlayersReplicatedOnThisClient All Pawn Loading Yet"));
			return false;
		}
	}

	return true;
}

void APDPlayerController::Client_SetExpectedPlayerCount_Implementation(int32 InExpectedCount)
{
	ExpectedPlayerCount = InExpectedCount;
	UE_LOG(LogTemp, Log, TEXT("Expected Player Count Set to: %d"), ExpectedPlayerCount);
}

void APDPlayerController::ServerRPC_ReportClientReady_Implementation()
{
	APDPlayerState* PS = GetPlayerState<APDPlayerState>();
	if (PS)
	{
		PS->bClientReady = true;

		UE_LOG(LogTemp, Log, TEXT("Server: Player %s reported Ready!"), *PS->GetPlayerName());

		if (APDGameModeBase* GM = GetWorld()->GetAuthGameMode<APDGameModeBase>())
		{
			GM->CheckAllPlayersReady();
		}
	}
}

void APDPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (!IsLocalController())
	{
		return;
	}

}

void APDPlayerController::Client_OnGameStarted_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Client_OnGameStarted"));
	if (PlayerHUDClass && !PlayerHUDWidget)
	{
		PlayerHUDWidget = CreateWidget<UIngameHUD>(this, PlayerHUDClass);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Log, TEXT("Client_OnGameStarted World Null"));
		return;
	}

	APDGameStateBase* GS = World->GetGameState<APDGameStateBase>();
	APDPlayerState* LocalPDPlayerState = GetPlayerState<APDPlayerState>();
	if (!GS || !LocalPDPlayerState)
	{
		if (LoadingHUD)
		{
			LoadingHUD->RemoveFromParent();
			LoadingHUD = nullptr;
		}

		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->AddToViewport();
		}

		return;
	}

	const ETeamType LocalTeamID = LocalPDPlayerState->GetTeamID();
	bool bBoundTeamMateSlot = false;

	if (UPDInventoryComponent* InventoryComponent = LocalPDPlayerState->GetInventoryComponent())
	{
		InitGoldDisplay(InventoryComponent->GetGold());
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		APDPlayerState* PDPS = Cast<APDPlayerState>(PS);
		if (!PDPS)
		{
			continue;
		}

		const bool bIsMe = (PDPS == LocalPDPlayerState);
		const bool bIsMyTeam = (PDPS->GetTeamID() == LocalTeamID);

		if (APDPawnBase* TargetPawn = Cast<APDPawnBase>(PDPS->GetPawn()))
		{
			if (UPDPlayerUIComponent* UIComp = TargetPawn->GetUIComponent())
			{
				UIComp->InitComponents(TargetPawn, TargetPawn->GetWidgetComponent(), PDPS->GetPDAttributeSetBase());
				UIComp->SetPlayerNickName(PDPS->GetDisplayName(), LocalTeamID, PDPS->GetTeamID());
			}
		}

		if (!PlayerHUDWidget)
		{
			continue;
		}

		if (bIsMe)
		{
			PlayerHUDWidget->BindSlot(PDPS->GetDisplayName(), EHPBarSlot::Player, PDPS->GetPDAttributeSetBase(), LocalTeamID, PDPS->GetTeamID());
		}
		else if (bIsMyTeam && !bBoundTeamMateSlot)
		{
			PlayerHUDWidget->BindSlot(PDPS->GetDisplayName(), EHPBarSlot::Team1, PDPS->GetPDAttributeSetBase(), LocalTeamID, PDPS->GetTeamID());
			bBoundTeamMateSlot = true;
		}
	}

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->OnShopUI();
	}

	if (LoadingHUD)
	{
		LoadingHUD->RemoveFromParent();
		LoadingHUD = nullptr;
	}
 
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->RefreshTeamScoreColors();
		PlayerHUDWidget->AddToViewport();
	}
}

