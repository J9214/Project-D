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

    void AddScore(ETeamType Team, int32 Points);
    void SetBallHolder(APDPlayerState* NewHolder);
    void SetGoalInstigator(APDPlayerState* NewInstigator);
    void GoalScored();

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
    int32 TeamOneScore;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
    int32 TeamTwoScore;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
    int32 TeamThreeScore;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ball")
    class APDPlayerState* CurrentBallHolder;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ball")
    class APDPlayerState* GoalInstigator;
  
private:
    UPROPERTY(EditAnywhere);
    int32 BallHoldScore;

    UPROPERTY(EditAnywhere);
    int32 GoalHoldScore;
};
