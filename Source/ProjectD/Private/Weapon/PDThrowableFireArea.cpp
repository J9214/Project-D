#include "Weapon/PDThrowableFireArea.h"
#include "Components/SphereComponent.h"
#include "DataAssets/Weapon/DataAsset_Throwable.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "PDGameplayTags.h"

APDThrowableFireArea::APDThrowableFireArea()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
    
    bReplicates = true;
    SetReplicateMovement(false);

    Collision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
    RootComponent = Collision;
    
    Collision->InitSphereRadius(350.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APDThrowableFireArea::InitFromData(AActor* InOwnerActor, UDataAsset_Throwable* InData)
{
    OwnerActor = InOwnerActor;
    Data = InData;

    const float Radius = Data ? Data->FireRadius : 350.f;
    Collision->SetSphereRadius(Radius);
}

void APDThrowableFireArea::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetNetMode() != NM_DedicatedServer)
    {
        SpawnFireVFX();
    }

    if (!HasAuthority() || !Data || !OwnerActor)
    {
        return;
    }
    
    TArray<AActor*> AlreadyOverlapping;
    Collision->GetOverlappingActors(AlreadyOverlapping, APawn::StaticClass());

    for (AActor* Actor : AlreadyOverlapping)
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
        {
            OverlappingASCs.Add(TargetASC);
        }
    }
    
    Collision->OnComponentBeginOverlap.AddDynamic(this, &APDThrowableFireArea::OnSphereBeginOverlap);
    Collision->OnComponentEndOverlap.AddDynamic(this, &APDThrowableFireArea::OnSphereEndOverlap);

    const float TickInterval = Data->FireTickInterval > 0.f ? Data->FireTickInterval : 0.25f;
    const float Duration = Data->FireDuration > 0.f ? Data->FireDuration : 6.0f;

    GetWorldTimerManager().SetTimer(TickTimerHandle, this, &APDThrowableFireArea::TickDamage, TickInterval, true);
    GetWorldTimerManager().SetTimer(EndTimerHandle, this, &APDThrowableFireArea::EndArea, Duration, false);
}

void APDThrowableFireArea::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupFireVFX();
    
    if (HasAuthority())
    {
        GetWorldTimerManager().ClearTimer(TickTimerHandle);
        GetWorldTimerManager().ClearTimer(EndTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

void APDThrowableFireArea::OnSphereBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!HasAuthority() || !OtherActor)
    {
        return;
    }

    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
    if (!TargetASC)
    {
        return;
    }

    OverlappingASCs.Add(TargetASC);
}

void APDThrowableFireArea::OnSphereEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    if (!HasAuthority() || !OtherActor)
    {
        return;
    }

    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
    if (!TargetASC)
    {
        return;
    }

    OverlappingASCs.Remove(TargetASC);
}

void APDThrowableFireArea::TickDamage()
{
    if (!HasAuthority() || !Data || !OwnerActor || !Data->FireDamageGE)
    {
        return;
    }
    
    CleanupInvalidTargets();
    
    if (OverlappingASCs.Num() <= 0)
    {
        return;
    }
    
    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
    if (!SourceASC)
    {
        return;
    }

    FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
    Context.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Data->FireDamageGE, 1.0f, Context);
    if (!SpecHandle.IsValid())
    {
        return;
    }

    const float Damage = Data->FireDamagePerTick;
    SpecHandle.Data->SetSetByCallerMagnitude(PDGameplayTags::Data_Throwable_Flame_Damage, Damage);

    for (auto It = OverlappingASCs.CreateIterator(); It; ++It)
    {
        UAbilitySystemComponent* TargetASC = It->Get();
        if (!TargetASC)
        {
            It.RemoveCurrent();
            continue;
        }

        SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    }
}

void APDThrowableFireArea::EndArea()
{
    if (HasAuthority())
    {
        CleanupFireVFX();
        Destroy();
    }
}

void APDThrowableFireArea::CleanupInvalidTargets()
{
    for (auto It = OverlappingASCs.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
        }
    }
}

void APDThrowableFireArea::SpawnFireVFX()
{
    if (SpawnedFireVFX || !FireLoopVFX)
    {
        return;
    }

    SpawnedFireVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
        FireLoopVFX,
        RootComponent,
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true,
        true
    );
    
    if (!SpawnedFireVFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn fire VFX"));
        return;
    }
    
    const float Radius = (Data ? Data->FireRadius : 350.f);
    SpawnedFireVFX->SetFloatParameter(TEXT("FireRadius"), Radius);
}

void APDThrowableFireArea::CleanupFireVFX()
{
    if (SpawnedFireVFX)
    {
        SpawnedFireVFX->Deactivate();
        SpawnedFireVFX->DestroyComponent();
        SpawnedFireVFX = nullptr;
    }
}
