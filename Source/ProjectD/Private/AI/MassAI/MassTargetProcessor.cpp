// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MassAI/MassTargetProcessor.h"
#include "AI/MassAI/MassTargetFragment.h"
#include "MassExecutionContext.h"
#include "AI/MassAI/MassBoidsProcessor.h"

// [Test] - To Chase Player
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

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
    // [Test] - 현재는 0번째 플레이어가 추적 대상
    FVector PlayerLocation = FVector::ZeroVector;
    bool bFoundPlayer = false;

    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
            bFoundPlayer = true;
        }
    }

    EntityQuery.ForEachEntityChunk(Context, [this, PlayerLocation, bFoundPlayer](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            TArrayView<FMassTargetFragment> TargetInfos = Context.GetMutableFragmentView<FMassTargetFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FMassTargetFragment& TargetInfo = TargetInfos[i];
                if (bFoundPlayer)
                {
                    TargetInfo.TargetPosition = PlayerLocation;
                    TargetInfo.IsTargetChase = true;
                }
                else
                {
                    TargetInfo.IsTargetChase = false;
                }
            }
        });
}
