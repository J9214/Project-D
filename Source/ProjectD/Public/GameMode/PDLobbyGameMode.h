// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interface/PDTeamInterface.h"
#include "PDLobbyGameMode.generated.h"

struct FUniqueNetIdRepl;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FTeamInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ETeamType TeamID = ETeamType::None;

	UPROPERTY(BlueprintReadOnly)
	FString LeaderSteamId;

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 PendingCount = 0;
};

UCLASS()
class PROJECTD_API APDLobbyGameMode : public AGameModeBase
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

	void TryGameStart(bool bIsTest);

private:
	static constexpr uint8 TEAM_COUNT = static_cast<uint8>(ETeamType::MAX); 

	FTeamInfo TeamInfos[TEAM_COUNT];
	const int32 MaxTeamSize = 1;

	TMap<FString, FString> LoginInfo;

	int32 PendingIncomingPlayers;

	bool bIsSessionCreating = false;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

};
