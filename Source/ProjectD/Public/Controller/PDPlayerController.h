#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PDPlayerController.generated.h"

class APDWeaponBase;
class UUserWidget;
class UWeaponManageComponent;

UCLASS()
class PROJECTD_API APDPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
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
