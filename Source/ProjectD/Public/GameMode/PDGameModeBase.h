#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PDGameModeBase.generated.h"

class APDPlayerState;
class ABallCore;
class AGoalPost;

UENUM(BlueprintType)
enum class ERoundPhase : uint8
{
	Waiting,
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
	void HandleGoalScored(AGoalPost* GoalPost, ABallCore* Ball);

protected:
	void PlayerRespawn(AController* Controller);

	void StartRound();
	void OnRoundTick();
	void HandleRoundEnd();
	void StartMatchFlow();
	void PrepareNextRound();

	int32 CalculateBestTeamId(bool& bOutTie) const;
	bool IsLastRound() const;

	void CacheRoundActors();
	void CachePlacedGoalPosts();
	void SpawnAndCacheBallCore();

	void ResetRoundState();
	void ResetPlacedGoalPostsForRound();
	void ResetBallForRound();

	FVector CalculateBallSpawnLocationFromGoals() const;
	FVector BuildRespawnLocationForController(AController* Controller) const;
	FVector BuildRespawnLocationFromTeam(int32 TeamId) const;
    
	UFUNCTION()
	void OnPlayerOutOfHealth(AController* VictimController, AActor* DamageCauser);

protected:
	FTimerHandle RoundTimerHandle;
	FTimerHandle NextRoundTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
	int32 RoundDurationSec;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
	int32 MaxRoundCount;

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
};
