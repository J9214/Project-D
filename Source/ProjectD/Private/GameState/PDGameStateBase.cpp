#include "GameState/PDGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/PDPlayerState.h"

APDGameStateBase::APDGameStateBase()
{
    TeamOneScore = 0;
    TeamTwoScore = 0;
    TeamThreeScore = 0;
    CurrentBallHolder = nullptr;
    GoalInstigator = nullptr;
    BallHoldScore = 50;
	GoalHoldScore = 500;
}

void APDGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APDGameStateBase, TeamOneScore);
    DOREPLIFETIME(APDGameStateBase, TeamTwoScore);
    DOREPLIFETIME(APDGameStateBase, TeamThreeScore);
    DOREPLIFETIME(APDGameStateBase, CurrentBallHolder);
    DOREPLIFETIME(APDGameStateBase, GoalInstigator);
}

void APDGameStateBase::AddScore(ETeamType Team, int32 Points)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    if (Team == ETeamType::TeamOne) TeamOneScore += Points;
    else if (Team == ETeamType::TeamTwo) TeamTwoScore += Points;
    else if (Team == ETeamType::TeamThree) TeamThreeScore += Points;

    // UE_LOG(LogTemp, Warning, TEXT("Team1: %d  |  Team2: %d  |  Team3: %d"), TeamOneScore, TeamTwoScore, TeamThreeScore);
}

void APDGameStateBase::SetBallHolder(APDPlayerState* NewHolder)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    if (!NewHolder)
    {
        return;
    }

    if (CurrentBallHolder != nullptr)
    {
        return;
    }

    CurrentBallHolder = NewHolder;

    ETeamType Team = CurrentBallHolder->GetTeamID();
    AddScore(Team, BallHoldScore);
}

void APDGameStateBase::SetGoalInstigator(APDPlayerState* NewInstigator)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

	GoalInstigator = NewInstigator;
}

void APDGameStateBase::GoalScored()
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

	ETeamType Team = CurrentBallHolder->GetTeamID();

	AddScore(Team, GoalHoldScore);
}

