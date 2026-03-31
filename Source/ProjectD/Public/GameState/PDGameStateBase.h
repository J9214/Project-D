#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Interface/PDTeamInterface.h"
#include "PDGameStateBase.generated.h"

class APDPlayerState;

UCLASS()
class PROJECTD_API APDGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
    APDGameStateBase();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void InitScores();
    void AddScore(ETeamType Team, int32 Points);
    void SetBallHolder(APDPlayerState* NewHolder);
    void SetGoalInstigator(APDPlayerState* NewInstigator);
    void GoalScored();

    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetScoreByTeam(ETeamType Team) const;

    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetScoreByTeamNumber(int32 TeamNumber) const;
    
    UFUNCTION()
    void OnRep_RemainingTime();
    
	UFUNCTION()
	void OnRep_ChangeWinnerTeamId();
    
    // Blueprint HUD reads team scores using 1-based display indices (Team1=1, Team2=2, Team3=3).
    // Index 0 is reserved so existing BP bindings stay aligned with the visible team numbers.
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
	TArray<int32> TeamScores;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ball")
    class APDPlayerState* CurrentBallHolder;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ball")
    class APDPlayerState* GoalInstigator;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, BlueprintReadOnly);
    int32 RemainingTimeSec;

    UPROPERTY(ReplicatedUsing = OnRep_ChangeWinnerTeamId, VisibleAnywhere, BlueprintReadOnly, Category = "Winner")
    int32 WinnerTeamId;

    UPROPERTY(Replicated, VIsibleAnyWhere, BlueprintReadOnly)
	bool bOvertime;
  
private:
    static constexpr int32 TeamScoreArrayOffset = 1;

    UPROPERTY(EditAnywhere);
    int32 BallHoldScore;

    UPROPERTY(EditAnywhere);
    int32 GoalHoldScore;

    UPROPERTY()
    int32 TeamCount;
};
