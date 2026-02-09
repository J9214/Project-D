// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/PDGameModeBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PDLobbyGameMode.generated.h"

struct FUniqueNetIdRepl;
/**
 * 
 */
UCLASS()
class PROJECTD_API APDLobbyGameMode : public APDGameModeBase
{
	GENERATED_BODY()
	
public:
	APDLobbyGameMode();

protected:
	virtual void BeginPlay() override;

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void CreateDedicatedSession();

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void UpdateSessionMetadata();

private:

	int32 TeamCounts[3];
	const int32 MaxTeamSize = 3;

	int32 PendingIncomingPlayers;

	bool bIsSessionCreating = false;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
};
