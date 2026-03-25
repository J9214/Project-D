#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PDGameModeBase.generated.h"

class APDPlayerState;
class ABallCore;
class AGoalPost;
class ADroneSpawner;
class ABallSpawnPosition;

UENUM(BlueprintType)
enum class ERoundPhase : uint8
{
	Waiting,
	InitPreRound,
	InRound,
	RoundEnded,
	GameEnded
};

UCLASS()
class PROJECTD_API APDGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	APDGameModeBase();
	
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	void PlayerDied(AController* Controller);

	void StartOvertime();
	void FinishGame(int32 BestTeamId);

	void HandleBallPickedUp(APDPlayerState* HolderPlayerState, ABallCore* Ball);
	void HandleGoalEntered(AGoalPost* GoalPost, ABallCore* Ball);
	void HandleGoalScored(AGoalPost* GoalPost, ABallCore* Ball);

public:
	// GM Spawn BallCore
	// If Need To LocalSide, BallCore Move To GS
	FORCEINLINE const ABallCore* GetBallCore_Server() const {	return CachedBallCore; }

protected:
	void PlayerRespawn(AController* Controller);

	void StartInitialPreRound();
	void FinishInitialPreRound();
	void StartFirstRound();

	void StartRound();
	void StartMatchFlow();
	void PrepareNextRound();

	void StartGameTimer();
	void OnGameTick();
	void HandleGameTimeExpired();

	int32 CalculateBestTeamId(bool& bOutTie) const;

	void CacheRoundActors();
	void CachePlacedGoalPosts();
	void CachePlacedBallSpawnPositions();
	void SpawnAndCacheBallCore();
	void CacheDroneSpawner();

	void ResetRoundState();
	void ResetPlacedGoalPostsForRound();
	void ResetBallForRound();

	FVector GetRandomBallSpawnLocation() const;
	FVector BuildRespawnLocationForController(AController* Controller) const;
	FVector BuildRespawnLocationFromTeam(int32 TeamId) const;
    
	bool TryFinishGameByScoreCondition();

	void TriggerDroneSpawnOnBallPickup(APDPlayerState* HolderPlayerState);
	void TriggerDroneExplosionOnGoal();

	void TeleportAllPlayersToRoundStart();
	void TeleportPlayerToRoundStart(AController* Controller);

	void SetAllPlayersMovementLocked(bool bLocked);
	void SetPlayerMovementLocked(AController* Controller, bool bLocked);

	void SetAllPlayersShopEnabled(bool bEnabled);
	void SetPlayerShopEnabled(AController* Controller, bool bEnabled);

	UFUNCTION()
	void OnPlayerOutOfHealth(AController* VictimController, AActor* DamageCauser);

protected:
	FTimerHandle GameTimerHandle;
	FTimerHandle NextRoundTimerHandle;
	FTimerHandle InitialPreRoundTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
	int32 TargetScoreToWin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
	int32 TotalGameDurationSec;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
	float InitialPreRoundDurationSec;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
	float NextRoundDelaySec;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	float TeamRespawnRadiusFromBall;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	float RespawnHeightOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<ABallCore> BallCoreClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
	int32 CurrentRoundIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Rules")
	ERoundPhase RoundPhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	FVector CurrentRoundBallSpawnLocation;

	UPROPERTY()
	TObjectPtr<ABallCore> CachedBallCore;

	UPROPERTY()
	TArray<TObjectPtr<AGoalPost>> CachedGoalPosts;

	UPROPERTY()
	TArray<TObjectPtr<ABallSpawnPosition>> CachedBallSpawnPositions;

	UPROPERTY()
	TObjectPtr<ADroneSpawner> CachedDroneSpawner;

	bool bDroneSpawnTriggeredThisRound = false;
	bool bGoalProcessingThisRound = false;

	bool bInitialPreRoundFinished = false;
	bool bGameTimerStarted = false;
};
