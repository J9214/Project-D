#include "GameMode/PDGameModeBase.h"
#include "PlayerState/PDPlayerState.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "GameState/PDGameStateBase.h"

APDGameModeBase::APDGameModeBase()
{
}

void APDGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	const UWorld* World = GetWorld();
	UE_LOG(LogTemp, Warning, TEXT("[Server GM] Map=%s GM=%s DefaultPawnClass=%s"),
		World ? *World->GetMapName() : TEXT("None"),
		*GetNameSafe(GetClass()),
		*GetNameSafe(DefaultPawnClass));

	RoundDurationSec = 300;


	StartRound();
}

void APDGameModeBase::PlayerDied(AController* Controller)
{
    if (!Controller) 
    {
        return;
    }

    if (APDPlayerState* PS = Controller->GetPlayerState<APDPlayerState>())
    {
        UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();

        if (ASC && GE_DeathClass)
        {
            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
            Context.AddSourceObject(this); 

            ASC->ApplyGameplayEffectToSelf(GetDefault<UGameplayEffect>(GE_DeathClass),1.0f,Context);
        }
    }

    FTimerHandle TimerHandle;
    FTimerDelegate TimerDel;
    TimerDel.BindUObject(this, &APDGameModeBase::PlayerRespawn, Controller);
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TimerHandle, TimerDel, 5.0f, false);
    }
}

void APDGameModeBase::StartOvertime()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (!GS) 
    {
        return;
    }

    GS->bOvertime = true;
}

void APDGameModeBase::FinishGame(int32 WinnerTeamId)
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();

    if(!GS) 
    {
        return;
	}

    if (GS->WinnerTeamId != INDEX_NONE)
    {
        return;
    }
    
    GS->WinnerTeamId = WinnerTeamId;
    GS->bOvertime = false;
}

void APDGameModeBase::PlayerRespawn(AController* Controller)
{
    if (!Controller) 
    {
        return;
    }

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn) 
    {
        return;
    }

    AActor* StartSpot = FindPlayerStart(Controller);
    if (StartSpot)
    {
        Pawn->TeleportTo(StartSpot->GetActorLocation(), StartSpot->GetActorRotation());
    }

    if (APDPlayerState* PS = Controller->GetPlayerState<APDPlayerState>())
    {
        UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
        if (ASC && GE_DeathClass && GE_ReviveClass)
        {
            ASC->RemoveActiveGameplayEffectBySourceEffect(GE_DeathClass, ASC);

            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();

            const UGameplayEffect* ReviveGE = GE_ReviveClass->GetDefaultObject<UGameplayEffect>();
            if (ReviveGE)
            {
                ASC->ApplyGameplayEffectToSelf(ReviveGE, 1.0f, Context);
            }
        }
    }
}

void APDGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (APDPlayerState* PS = NewPlayer->GetPlayerState<APDPlayerState>())
    {
        if (UPDAttributeSetBase* AS = PS->GetPDAttributeSetBase())
        {
            AS->OnOutOfHealth.AddDynamic(this, &APDGameModeBase::OnPlayerOutOfHealth);
        }
    }
}

void APDGameModeBase::OnPlayerOutOfHealth(AController* VictimController, AActor* DamageCauser)
{
    PlayerDied(VictimController);
}

void APDGameModeBase::StartRound()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (!GS) 
    {
        return;
    }

    GS->RemainingTimeSec = RoundDurationSec;

    GetWorldTimerManager().SetTimer(
        RoundTimerHandle,
        this,
        &APDGameModeBase::OnRoundTick,
        1.0f,
        true
    );
}

void APDGameModeBase::OnRoundTick()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (!GS) 
    {
        return;
    }

    GS->RemainingTimeSec = FMath::Max(0, GS->RemainingTimeSec - 1);

    if (GS->RemainingTimeSec <= 0)
    {
        GetWorldTimerManager().ClearTimer(RoundTimerHandle);
        HandleRoundEnd();
    }
}

void APDGameModeBase::HandleRoundEnd()
{
    APDGameStateBase* GS = GetGameState<APDGameStateBase>();
    if (!GS) 
    {
        return;
    }

    int32 BestTeamId = INDEX_NONE;
    int32 BestScore = MIN_int32;
    bool bTie = false;

    const int32 TeamCount = static_cast<int32>(ETeamType::MAX);

    for (int32 TeamId = 0; TeamId < GS->TeamScores.Num(); ++TeamId)
    {
        const int32 Score = GS->TeamScores[TeamId];
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTeamId = TeamId;
            bTie = false;
        }
        else if (Score == BestScore)
        {
            bTie = true;
        }
    }

    if (bTie)
    {
        StartOvertime();
    }
    else
    {
        FinishGame(BestTeamId);
    }
}
