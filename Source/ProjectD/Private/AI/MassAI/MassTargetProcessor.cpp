// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassTargetProcessor.h"
#include "AI/MassAI/MassTargetFragment.h"
#include "MassExecutionContext.h"
#include "AI/MassAI/MassBoidsProcessor.h"
#include "GameMode/PDGameModeBase.h"
#include "Object/BallCore.h"

UMassTargetProcessor::UMassTargetProcessor()
	:EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
    ExecutionOrder.ExecuteBefore.Add(UMassBoidsProcessor::StaticClass()->GetFName());
}

void UMassTargetProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMassTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    FVector BallLocation = FVector::ZeroVector;
    bool bHasBall = false;

    UWorld* World = GetWorld();
    if (IsValid(World) == true)
    {
        APDGameModeBase* GM = World->GetAuthGameMode<APDGameModeBase>();
        if (IsValid(GM) == true)
        {
            const ABallCore* BallCore = GM->GetBallCore_Server();
            if (IsValid(BallCore) == true)
            {
                BallLocation = BallCore->GetActorLocation();
                bHasBall = true;
            }
        }
        else
        {
            return;
        }
    }

    EntityQuery.ForEachEntityChunk(Context, [BallLocation, bHasBall](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            TArrayView<FMassTargetFragment> TargetInfos = Context.GetMutableFragmentView<FMassTargetFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FMassTargetFragment& TargetInfo = TargetInfos[i];

                if (bHasBall == true)
                {
                    TargetInfo.TargetPosition = BallLocation;
                    TargetInfo.IsTargetChase = true;
                }
                else
                {
                    TargetInfo.IsTargetChase = false;
                }
            }
        });
}
