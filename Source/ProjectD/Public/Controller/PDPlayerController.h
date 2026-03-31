#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/Shop/FPDItemInfo.h"
#include "Interface/PDTeamInterface.h"
#include "PDPlayerController.generated.h"

class APDWeaponBase;
class UUserWidget;
class UWeaponManageComponent;
class UPDShopComponent;
class UIngameHUD;
class UPDAttributeSetBase;

UCLASS()
class PROJECTD_API APDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APDPlayerController();

	UFUNCTION(Exec)
	void KillSelfCheat();

	UFUNCTION(Exec)
	void ReviveSelfCheat();
	
	FORCEINLINE UPDShopComponent* GetShopComponent() const { return ShopComponent; }

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void ShowGameOver();

	void InitGoldDisplay(int InGold);
	void InitItemDataDisplay(EItemType ItemType, int SlotIndex, const FName& NewItemID, int Count);

	UIngameHUD* GetIngameHUDWidget() const { return PlayerHUDWidget; }

	void InitializeHUD();

	void UpdateCurrentAmmo(int32 CurrentAmmo);

	UFUNCTION(Client, Reliable)
	void Client_OnGameStarted();

protected:

	void StartReadyCheck();
	void TickReadyCheck();
	bool AreAllPlayersReplicatedOnThisClient() const;

	FTimerHandle ReadyCheckTimerHandle;
	int32 ExpectedPlayerCount = 0;
	bool bLocalReadyReported = false;

	UFUNCTION(Server, Reliable)
	void ServerRPC_ReportClientReady();

public:
	UFUNCTION(Client, Reliable)
	void Client_SetExpectedPlayerCount(int32 InExpectedCount);

protected:

	UFUNCTION(Server, Reliable)
	void Server_KillSelfCheat();

	UFUNCTION(Server, Reliable)
	void Server_ReviveSelfCheat();

	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY()
	TObjectPtr<UIngameHUD> PlayerHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingHUDClass;

	UPROPERTY()
	UUserWidget* LoadingHUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> ResultWidgetClass;
	UPROPERTY()
	UUserWidget* ResultWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<UPDShopComponent> ShopComponent;

private:
	UFUNCTION(BlueprintCallable)
	void OnAimStarted();

	UFUNCTION(BlueprintCallable)
	void OnAimEnded();
	
	void ShowAimCrosshair(TSubclassOf<UUserWidget> CrosshairClass);
	void HideAimCrosshair();

	TSubclassOf<UUserWidget> GetCurrentAimCrosshairClass() const;

	void HandleWeaponChanged(APDWeaponBase* NewWeapon);
	
	void BindWeaponChangedDelegate();
	void UnbindWeaponChangedDelegate() const;
	
private:
	UPROPERTY()
	TSubclassOf<UUserWidget> CurrentCrosshairClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentCrosshairWidget;
	
	bool bWantsToAim = false;
};
