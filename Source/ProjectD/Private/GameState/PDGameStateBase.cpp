#include "GameState/PDGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/PDPlayerState.h"
#include "GameMode/PDGameModeBase.h"
#include "Controller/PDPlayerController.h"

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

    TeamScores.Init(0, TeamCount);
    
    for(int32 &TeamScore : TeamScores)
    {
        TeamScore = 0;
	}
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

	const int32 TeamId = static_cast<int32>(Team);

    if (TeamScores.IsValidIndex(static_cast<int32>(TeamId)))
    {
        TeamScores[TeamId] += Points;
    }

    if (bOvertime && Points > 0)
    {
        if (APDGameModeBase* GM = GetWorld()->GetAuthGameMode<APDGameModeBase>())
        {
            GM->FinishGame(TeamId);
        }
    }

    /*for(auto Score : TeamScores)
    {
        UE_LOG(LogTemp, Warning, TEXT("Score: %d"), Score);
	}*/
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

	ETeamType Team = CurrentBallHolder->GetTeamID();

	AddScore(Team, GoalHoldScore);
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

