// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PDGameInstance.generated.h"

class UCreateSessionCallbackProxyAdvanced;

/**
 * 
 */
UCLASS()
class PROJECTD_API UPDGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()
	
public:

	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void HostPartySessionCreate();

	UFUNCTION(BlueprintCallable)
	void TravelToDedicated(const FString& DediAddress);

private:

	UPROPERTY()
	TObjectPtr<UCreateSessionCallbackProxyAdvanced> CreateSessionProxy;

	IOnlineSessionPtr SessionInterface;
	FDelegateHandle InviteAcceptedHandle;
	FDelegateHandle JoinCompleteHandle;
	FDelegateHandle DestroyCompleteHandle;
	FString PendingTravelURL;
	bool bPendingTravelToDedi = false;


	UFUNCTION()
	void OnCreateHostSessionSuccess();

	UFUNCTION()
	void OnCreateHostSessionFailure();

	UFUNCTION()
	void BindSessionDelegates();

	void HandleInviteAccepted(const bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

};
