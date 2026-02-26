#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interface/PDTeamInterface.h"
#include "PDPlayerController.generated.h"

class APDWeaponBase;
class UUserWidget;
class UWeaponManageComponent;
class UPDShopComponent;
class UIngameHUD;

UCLASS()
class PROJECTD_API APDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	FORCEINLINE UPDShopComponent* GetShopComponent() const { return ShopComponent; }

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void ShowGameOver();

	void InitGoldDisplay(int InGold);
	void InitItemDataDisplay();

	UIngameHUD* GetIngameHUDWidget() const { return PlayerHUDWidget; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY()
	TObjectPtr<UIngameHUD> PlayerHUDWidget;

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
