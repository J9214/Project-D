#include "GameState/PDGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/PDPlayerState.h"
#include "GameMode/PDGameModeBase.h"
#include "Controller/PDPlayerController.h"

namespace
{
constexpr int32 TeamScoreArrayOffset = 1;

int32 GetScoreArrayIndexFromTeam(const ETeamType Team)
{
    const int32 TeamId = static_cast<int32>(Team);
    const int32 PlayableTeamCount = static_cast<int32>(ETeamType::MAX);

    if (TeamId < 0 || TeamId >= PlayableTeamCount)
    {
        return INDEX_NONE;
    }

    return TeamId + TeamScoreArrayOffset;
}
}

APDGameStateBase::APDGameStateBase()
{
	TeamCount = static_cast<int32>(ETeamType::MAX);
    InitScores();
    CurrentBallHolder = nullptr;
    GoalInstigator = nullptr;
    BallHoldScore = 50;
	GoalHoldScore = 500;
    WinnerTeamId = INDEX_NONE;
}

void APDGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APDGameStateBase, TeamScores);
    DOREPLIFETIME(APDGameStateBase, CurrentBallHolder);
    DOREPLIFETIME(APDGameStateBase, GoalInstigator);
    DOREPLIFETIME(APDGameStateBase, bOvertime);
    DOREPLIFETIME(APDGameStateBase, RemainingTimeSec);
    DOREPLIFETIME(APDGameStateBase, WinnerTeamId);
}

void APDGameStateBase::InitScores()
{
    if (!HasAuthority())
    {
        return;
    }

    TeamScores.Init(0, TeamCount + TeamScoreArrayOffset);
    
	for(int32 &TeamScore : TeamScores)
    {
        TeamScore = 0;
	}

    NotifyTeamScoresChanged();
}

void APDGameStateBase::AddScore(ETeamType Team, int32 Points)
{
    if (!HasAuthority())
    {
        return;
    }

	if (WinnerTeamId != INDEX_NONE)
    {
        return; 
    }

    const int32 ScoreIndex = GetScoreArrayIndexFromTeam(Team);

    if (TeamScores.IsValidIndex(ScoreIndex))
    {
        TeamScores[ScoreIndex] += Points;
        NotifyTeamScoresChanged();
    }
}

void APDGameStateBase::SetBallHolder(APDPlayerState* NewHolder)
{
    if (!HasAuthority())
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
    if (!HasAuthority())
    {
        return;
    }

	GoalInstigator = NewInstigator;
}

void APDGameStateBase::GoalScored()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!IsValid(GoalInstigator))
    {
        return;
    }

	ETeamType Team = GoalInstigator->GetTeamID();

	AddScore(Team, GoalHoldScore);
}

int32 APDGameStateBase::GetScoreByTeam(const ETeamType Team) const
{
    const int32 ScoreIndex = GetScoreArrayIndexFromTeam(Team);
    return TeamScores.IsValidIndex(ScoreIndex) ? TeamScores[ScoreIndex] : 0;
}

int32 APDGameStateBase::GetScoreByTeamNumber(const int32 TeamNumber) const
{
    return TeamScores.IsValidIndex(TeamNumber) ? TeamScores[TeamNumber] : 0;
}

void APDGameStateBase::NotifyTeamScoresChanged()
{
    TeamScoresChangedNative.Broadcast();
}

void APDGameStateBase::OnRep_TeamScores()
{
    NotifyTeamScoresChanged();
}

void APDGameStateBase::OnRep_RemainingTime()
{
    //UE_LOG(LogTemp, Log, TEXT("Remaining Time: %d"), RemainingTimeSec);
}

void APDGameStateBase::OnRep_ChangeWinnerTeamId()
{
    if (WinnerTeamId != INDEX_NONE)
    {
        UE_LOG(LogTemp, Log, TEXT("Winner Team Id: %d"), WinnerTeamId);
	}

    APDPlayerController* PC = GetWorld()->GetFirstPlayerController<APDPlayerController>();

    if (PC)
    {
        PC->ShowGameOver();
	}
}

