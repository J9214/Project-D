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