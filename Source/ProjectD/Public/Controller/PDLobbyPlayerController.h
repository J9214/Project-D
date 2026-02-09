// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PDLobbyPlayerController.generated.h"

class UUserWidget;
/**
 * 
 */
UCLASS()
class PROJECTD_API APDLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ConnectToDedicatedServer();

	UFUNCTION(Client, Reliable)
	void Client_TravelToTargetServer(const FString& ConnectString);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI, Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> UIWidgetInstance;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	int32 CurrentSessionIndex = 0;
	int32 MyTeamSize = 1;

	FString PendingSteamAddress;

	void TryJoinNextAvailableSession();
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	void OnFindSessionsComplete(bool bWasSuccessful);
};
