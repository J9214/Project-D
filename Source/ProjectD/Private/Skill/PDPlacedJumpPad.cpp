#include "Skill/PDPlacedJumpPad.h"

#include "AbilitySystemComponent.h"
#include "Components/Input/MovementBridgeComponent.h"
#include "Components/SphereComponent.h"
#include "Gimmick/Data/PDJumpPadDataAsset.h"
#include "MoverComponent.h"
#include "Pawn/PDPawnBase.h"
#include "PDGameplayTags.h"
#include "ProjectD/ProjectD.h"

APDPlacedJumpPad::APDPlacedJumpPad()
{
	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(GetRootComponent());
	TriggerSphere->SetSphereRadius(150.f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetGenerateOverlapEvents(true);
}

void APDPlacedJumpPad::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(TriggerSphere) && !TriggerSphere->OnComponentBeginOverlap.IsAlreadyBound(this, &ThisClass::HandlePadOverlap))
	{
		TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandlePadOverlap);
	}

	if (!IsValid(JumpPadDataAsset))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDPlacedJumpPad::BeginPlay - JumpPadDataAsset is not assigned."));
	}
}

void APDPlacedJumpPad::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(TriggerSphere) && TriggerSphere->OnComponentBeginOverlap.IsAlreadyBound(this, &ThisClass::HandlePadOverlap))
	{
		TriggerSphere->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::HandlePadOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

void APDPlacedJumpPad::OnEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec,
	FActiveGameplayEffectHandle Handle)
{
	(void)ASC;
	(void)Handle;

	const float DamageAmount = ExtractDamageFromSpec(Spec);
	const FGameplayEffectContextHandle& Context = Spec.GetContext();
	const FHitResult* Hit = Context.GetHitResult();
	const FHitResult EmptyHitResult;

	if (!HasAuthority() || DamageAmount <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
	OnHitEvent(DamageAmount, Hit ? *Hit : EmptyHitResult);

	if (CurrentHealth <= 0.f)
	{
		Destroy();
	}
}

void APDPlacedJumpPad::HandlePadOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	LaunchActorByPad(OtherActor);
}

void APDPlacedJumpPad::LaunchActorByPad(AActor* Interactor)
{
	if (!IsValid(JumpPadDataAsset))
	{
		return;
	}

	UMoverComponent* Mover = Interactor ? Interactor->GetComponentByClass<UMoverComponent>() : nullptr;
	if (!IsValid(Mover))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDPlacedJumpPad::LaunchActorByPad - Interactor has Invalid Mover!"));
		return;
	}

	APDPawnBase* PDPawn = Cast<APDPawnBase>(Interactor);
	if (!IsValid(PDPawn))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDPlacedJumpPad::LaunchActorByPad - Interactor is not PDPawnBase!"));
		return;
	}

	UMovementBridgeComponent* BridgeComp = PDPawn->GetMovementBridgeComponent();
	if (!IsValid(BridgeComp))
	{
		UE_LOG(LogProjectD, Warning, TEXT("APDPlacedJumpPad::LaunchActorByPad - Invalid MovementBridgeComponent!"));
		return;
	}

	FMoveRequest Request;
	Request.Type = EMoveRequestType::JumpTo;
	Request.bUseActorRotation = JumpPadDataAsset->JumpInfo.bUseActorRotation;
	Request.JumpHeight = JumpPadDataAsset->JumpInfo.JumpHeight;

	FVector Velocity = Mover->GetVelocity();
	Velocity.Z = 0.0f;
	Request.JumpDistance = Request.bUseActorRotation
		? JumpPadDataAsset->JumpInfo.JumpDistance + Velocity.Length()
		: JumpPadDataAsset->JumpInfo.JumpDistance;
	Request.JumpRotation = Request.bUseActorRotation
		? JumpPadDataAsset->JumpInfo.JumpRotation
		: GetActorRotation();
	Request.ForceMovementMode = "Falling";

	BridgeComp->EnqueueMoveRequest(Request);
}

float APDPlacedJumpPad::ExtractDamageFromSpec(const FGameplayEffectSpec& Spec) const
{
	float DamageAmount = 0.f;

	DamageAmount += Spec.GetSetByCallerMagnitude(PDGameplayTags::Data_Weapon_Damage, false, 0.f);
	DamageAmount += Spec.GetSetByCallerMagnitude(PDGameplayTags::Data_Throwable_Fragment_Damage, false, 0.f);
	DamageAmount += Spec.GetSetByCallerMagnitude(PDGameplayTags::Data_Throwable_Flame_Damage, false, 0.f);
	DamageAmount += Spec.GetSetByCallerMagnitude(PDGameplayTags::Data_ThrowableObject_ExplodeDamage, false, 0.f);
	DamageAmount += Spec.GetSetByCallerMagnitude(PDGameplayTags::Data_ThrowableObject_ContinuousDamage, false, 0.f);
	DamageAmount += Spec.GetSetByCallerMagnitude(PDGameplayTags::Data_AI_Drone_ExplodeDamage, false, 0.f);

	return DamageAmount;
}
