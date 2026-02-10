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
    
    UFUNCTION()
    void OnRep_RemainingTime();
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
	TArray<int32> TeamScores;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ball")
    class APDPlayerState* CurrentBallHolder;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ball")
    class APDPlayerState* GoalInstigator;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, BlueprintReadOnly);
    int32 RemainingTimeSec;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Winner")
    int32 WinnerTeamId;

    UPROPERTY(Replicated, BlueprintReadOnly)
	bool bOvertime;
  
private:
    UPROPERTY(EditAnywhere);
    int32 BallHoldScore;

    UPROPERTY(EditAnywhere);
    int32 GoalHoldScore;

    UPROPERTY()
    int32 TeamCount;
};
