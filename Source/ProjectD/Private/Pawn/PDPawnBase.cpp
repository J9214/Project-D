#include "Pawn/PDPawnBase.h"
#include "PlayerState/PDPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/PDAbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/Input/PDEnhancedInputComponent.h"
#include "Components/Combat/WeaponManageComponent.h"
#include "Components/Combat/WeaponStateComponent.h"
#include "Components/Combat/SkillManageComponent.h"
#include "Components/Input/MovementBridgeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "GameplayTagContainer.h"
#include "PDGameplayTags.h"
#include "DataAssets/StartUp/DataAsset_StartUpBase.h"
#include "AttributeSet/PDAttributeSetBase.h"
#include "Object/BallCore.h"
#include "Object/GoalPost.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Gimmick/PDInteractableObject.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h" 
#include "MoverComponent.h"
#include "Object/Throwable/PDThrowableObject.h"
#include "CollisionShape.h"
#include "Components/InteractionComponent.h"
#include "Weapon/PDWeaponBase.h"
#include "Structs/FSpeedUpModifier.h"
#include "Gimmick/ZipLine/PDZipLine.h"

APDPawnBase::APDPawnBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = true;

	WeaponManageComponent = CreateDefaultSubobject<UWeaponManageComponent>(TEXT("WeaponManageComponent"));
	WeaponStateComponent = CreateDefaultSubobject<UWeaponStateComponent>(TEXT("WeaponStateComponent"));
	SkillManageComponent = CreateDefaultSubobject<USkillManageComponent>(TEXT("SkillManageComponent"));
	MovementBridgeComponent = CreateDefaultSubobject<UMovementBridgeComponent>(TEXT("MovementBridgeComponent"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

	OverrideInputComponentClass = UPDEnhancedInputComponent::StaticClass();
}

UAbilitySystemComponent* APDPawnBase::GetAbilitySystemComponent() const
{
	if (APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>())
	{
		return PDPlayerState->GetAbilitySystemComponent();
	}

	return nullptr;
}

USkeletalMeshComponent* APDPawnBase::GetSkeletalMeshComponent() const
{
	return FindComponentByClass<USkeletalMeshComponent>();
}

void APDPawnBase::ClientDrawFireDebug_Implementation(
	const FVector& Start,
	const FVector& End,
	bool bHit,
	const FVector& HitPoint
)
{
	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.f, 0, 1.f);
	if (bHit)
	{
		DrawDebugPoint(GetWorld(), HitPoint, 8.f, FColor::Yellow, false, 1.f);
	}
}

void APDPawnBase::BeginPlay()
{
	Super::BeginPlay();

	MoverComponent = FindComponentByClass<UMoverComponent>();
	
	CachedCamera = FindComponentByClass<UCameraComponent>();
	CachedSpringArm = FindComponentByClass<USpringArmComponent>();

	if (CachedCamera)
	{
		CachedCamera->SetFieldOfView(ThirdPersonFOV);
	}

	if (CachedSpringArm)
	{
		SavedArmLength = CachedSpringArm->TargetArmLength;
		bSavedDoCollisionTest = CachedSpringArm->bDoCollisionTest;
	}
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UE_LOG(LogTemp, Warning, TEXT("ViewTarget=%s"), *GetNameSafe(PC->GetViewTarget()));
	}
}

void APDPawnBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
	
	if (HasAuthority())
	{
		if (!CharacterStartUpData.IsNull())
		{
			if (UDataAsset_StartUpBase* LoadedData = CharacterStartUpData.LoadSynchronous())
			{
				LoadedData->GiveToAbilitySystemComponent(Cast<UPDAbilitySystemComponent>(GetAbilitySystemComponent()));
			}
		}
		
		InitAttributeSet();
	}
}

void APDPawnBase::OnRep_PlayerState()
{
	InitAbilityActorInfo();
}

void APDPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->AddMappingContext(InputConfigDataAsset->AbilityIMC, InputConfigDataAsset->IMCPriority);
		}
	}

	if (UPDEnhancedInputComponent* PDInputComponent = Cast<UPDEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IsValid(InputConfigDataAsset))
		{
			PDInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, &ThisClass::Input_AbilityInputPressed, &ThisClass::Input_AbilityInputReleased);
		}
	}
}

void APDPawnBase::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (!bIsFirstPerson)
	{
		Super::CalcCamera(DeltaTime, OutResult);
		return;
	}

	APDWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon)
	{
		Super::CalcCamera(DeltaTime, OutResult);
		return;
	}

	const FTransform AnchorWorld = Weapon->GetSightCameraWorldTransform();

	const FRotator ViewRot = (Controller ? Controller->GetControlRotation() : AnchorWorld.Rotator());

	OutResult.Location = AnchorWorld.GetLocation() + AnchorWorld.GetRotation().RotateVector(FirstPersonOffsetLocal);
	OutResult.Rotation = ViewRot;
	OutResult.FOV = FirstPersonFOV;
}

void APDPawnBase::InitAbilityActorInfo()
{
	APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>();
	if (!PDPlayerState)
	{
		return;
	}

	PDPlayerState->InitAbilityActorInfo(this);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (ASC)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
		FDelegateHandle TagEventHandle = ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ThisClass::OnDeathTagChanged);
	}
	
}

void APDPawnBase::InitAttributeSet()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("APDPawnBase::InitAttributeSet - ASC is not valid"));
		return;
	}
	
	APDPlayerState* PDPlayerState = GetPlayerState<APDPlayerState>();
	if (!PDPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("APDPawnBase::InitAttributeSet - PlayerState is not valid"));
		return;
	}
	
	ASC->AddAttributeSetSubobject<UPDAttributeSetBase>(PDPlayerState->GetPDAttributeSetBase());
	
	BindAttributeChangeDelegates();
}

void APDPawnBase::BindAttributeChangeDelegates()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("APDPawnBase::InitAttributeSet - ASC is not valid"));
		return;
	}
	
	ASC->GetGameplayAttributeValueChangeDelegate(UPDAttributeSetBase::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
}

void APDPawnBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
}

void APDPawnBase::Input_AbilityInputPressed(FGameplayTag InputTag)
{
	if (UPDAbilitySystemComponent* ASC = Cast<UPDAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->OnAbilityInputPressed(InputTag);
	}
}

void APDPawnBase::Input_AbilityInputReleased(FGameplayTag InputTag)
{
	if (UPDAbilitySystemComponent* ASC = Cast<UPDAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->OnAbilityInputReleased(InputTag);
	}
}

void APDPawnBase::OnAimHoldStarted(const FInputActionValue& Value)
{
	bAimHoldDown = true;
	
	if (bIsFirstPerson)
	{
		SetIsAiming(true);
		return;
	}

	SetIsAiming(true);
}

void APDPawnBase::OnAimHoldEnded(const FInputActionValue& Value)
{
	bAimHoldDown = false;
	
	if (bIsFirstPerson)
	{
		SetIsAiming(true);   // 1인칭은 무조건 견착 유지
		return;
	}
	
	SetIsAiming(false);
}

void APDPawnBase::OnFirstPersonToggle(const FInputActionValue& Value)
{
	if (!bIsFirstPerson)
	{
		EnterFirstPerson();
	}
	else
	{
		ExitFirstPerson();
	}
}

void APDPawnBase::EnterFirstPerson()
{
	bIsFirstPerson = true;
	SetIsAiming(true);

	if (USkeletalMeshComponent* Body = GetSkeletalMeshComponent())
	{
		Body->SetOwnerNoSee(true);
	}
}

void APDPawnBase::ExitFirstPerson()
{
	bIsFirstPerson = false;
	SetIsAiming(bAimHoldDown);

	if (USkeletalMeshComponent* Body = GetSkeletalMeshComponent())
	{
		Body->SetOwnerNoSee(false);
	}
}

void APDPawnBase::UpdateFirstPersonCamera(float DeltaSeconds)
{
	if (!CachedCamera)
	{
		CachedCamera = FindComponentByClass<UCameraComponent>();
		if (!CachedCamera)
		{
			return;
		}
	}

	APDWeaponBase* Weapon = GetEquippedWeapon();
	if (!Weapon)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("APDPawnBase::UpdateFirstPersonCamera - Updating camera location for first-person view."));
	
	const FTransform AnchorWorld = Weapon->GetSightCameraWorldTransform();

	const FVector OffsetWorld = AnchorWorld.GetRotation().RotateVector(FirstPersonOffsetLocal);
	const FVector TargetLoc = AnchorWorld.GetLocation() + OffsetWorld;

	const FVector NewLocation = FMath::VInterpTo(
		CachedCamera->GetComponentLocation(),
		TargetLoc,
		DeltaSeconds,
		FirstPersonInterpSpeed
	);
	
	UE_LOG(LogTemp, Warning, TEXT("APDPawnBase::UpdateFirstPersonCamera - Target Location: %s, New Location: %s"), *TargetLoc.ToString(), *NewLocation.ToString());

	CachedCamera->SetWorldLocation(NewLocation);
}

APDWeaponBase* APDPawnBase::GetEquippedWeapon() const
{
	if (!WeaponManageComponent)
	{
		return nullptr;
	}
	
	return WeaponManageComponent->GetEquippedWeapon();
}

void APDPawnBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APDPawnBase, CarriedObject);
	DOREPLIFETIME(APDPawnBase, bIsAiming);
}

AActor* APDPawnBase::FindInteractTarget() const
{
	UCameraComponent* Cam = FindComponentByClass<UCameraComponent>();
	if (!Cam) 
	{
		return nullptr;
	}

	const FVector Start = Cam->GetComponentLocation();
	const FVector End = Start + Cam->GetForwardVector() * 450.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	constexpr float InteractTraceRadius = 30.f;
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(InteractTraceRadius),
		Params
	);

	FColor DebugColor = bHit ? FColor::Green : FColor::Red;

	/*DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 2.0f, 0, 1.5f);
	DrawDebugSphere(GetWorld(), Start, InteractTraceRadius, 16, FColor::Cyan, false, 2.0f, 0, 1.0f);
	DrawDebugSphere(GetWorld(), bHit ? Hit.ImpactPoint : End, InteractTraceRadius, 16, DebugColor, false, 2.0f, 0, 1.0f);*/

	return Hit.GetActor();
}

void APDPawnBase::OnDeathTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	const bool bIsDead = NewCount > 0;
	HandleDeathState(bIsDead);
}

void APDPawnBase::HandleDeathState(bool bIsDead)
{
	if (RootComponent)
	{
		if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(RootComponent))
		{
			Primitive->SetCollisionEnabled(bIsDead ? ECollisionEnabled::QueryOnly : ECollisionEnabled::QueryAndPhysics);
		}
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (bIsDead)
			DisableInput(PC);
		else
			EnableInput(PC);
	}
}

FVector APDPawnBase::GetDirectionByMoveInput(const FVector& FallbackForward) const
{
	if (!MoverComponent)
	{
		return FallbackForward;
	}
	
	const FMoverInputCmdContext& Cmd = MoverComponent->GetLastInputCmd();
	
	const FCharacterDefaultInputs* Inputs = Cmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();
	if (!Inputs)
	{
		return FallbackForward;
	}
	
	const FVector MoveInput= Inputs->GetMoveInput();
	if (MoveInput.IsNearlyZero())
	{
		return FallbackForward;
	}

	return MoveInput.GetSafeNormal();
}

void APDPawnBase::TryInteract()
{
	AActor* Target = FindInteractTarget();

	bool bIsInteractable = IsValid(Target) && Cast<APDDirectlyInteractGimmickBase>(Target);

	if (bIsInteractable)
	{
		if (Target->IsA(APDZipLine::StaticClass()))
		{
			IPDInteractableObject::Execute_OnInteract(Target, this);
		}
		else
		{
			Server_TryInteract(Target);
		}
	}
	else
	{
		if (IsValid(CarriedObject.Get()))
		{
			Server_DropObject(CalcCamDirection());
		}
	}
}

FVector APDPawnBase::CalcCamDirection()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!IsValid(PC))
	{
		return FVector::ForwardVector;
	}

	FVector CamLoc;
	FRotator CamRot;

	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector Forward = CamRot.Vector();
	
	return Forward.GetSafeNormal();;
}

void APDPawnBase::Server_ForceClearCarriedBall()
{
	if (!HasAuthority()) return;

	CarriedObject = nullptr;

	RemoveHoldingBallEffect();
}

void APDPawnBase::Server_TryInteract_Implementation(AActor* Target)
{
	if (Target && Cast<APDDirectlyInteractGimmickBase>(Target))
	{
		IPDInteractableObject::Execute_OnInteract(Target, this);
	}
}

void APDPawnBase::Server_PickUpObject_Implementation(APDCarriableObjectBase* Object)
{
	if (!Object || Object->CarrierPawn.Get())
	{
		return;
	}

	CarriedObject = Object;

	Object->SetCarrier(this);

	ApplyHoldingBallEffect();
}

void APDPawnBase::Server_DropObject_Implementation(const FVector& InCamDirecion)
{
	if (!IsValid(CarriedObject.Get()))
	{
		return;
	}

	RemoveHoldingBallEffect();

	const FVector Forward = GetActorForwardVector();
	const FVector DropLoc = GetActorLocation() + (Forward * 100.f) + FVector(0.f, 0.f, 80.f);

	const FVector Impulse = (Forward * 300.f) + (FVector::UpVector * 200.f);

	CarriedObject->DropPhysics(DropLoc, Impulse, InCamDirecion);

	CarriedObject = nullptr;
}

void APDPawnBase::ApplyHoldingBallEffect()
{
	UAbilitySystemComponent* ASC = APDPawnBase::GetAbilitySystemComponent();

	if (ASC && GE_HoldingBall)
	{
		ASC->ApplyGameplayEffectToSelf(GE_HoldingBall.GetDefaultObject(), 1.f, ASC->MakeEffectContext());
	}
}

void APDPawnBase::RemoveHoldingBallEffect()
{
	UAbilitySystemComponent* ASC = APDPawnBase::GetAbilitySystemComponent();

	if (ASC)
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.HoldingBall")));
		ASC->RemoveActiveEffectsWithGrantedTags(TagContainer);
	}
}

void APDPawnBase::CancelMovementGA()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("APDPawnBase::InitAttributeSet - ASC is not valid"));
		return;
	}

	FGameplayTag RootTag  = PDGameplayTags::Player_Ability_Movement;
	
	const TArray<FGameplayAbilitySpec>& ActiveSpecs = ASC->GetActivatableAbilities();
	
	FGameplayTagContainer TargetContainer;
	TargetContainer.AddTag(RootTag);
	
	for (const FGameplayAbilitySpec& Spec : ActiveSpecs)
	{
		if (Spec.IsActive())
		{
			if (Spec.Ability->GetAssetTags().HasTag(RootTag))
			{
				ASC->CancelAbilityHandle(Spec.Handle);
			}
		}
	}
}

float APDPawnBase::GetActiveSpeedUpMultiplier() const
{
	const UMoverComponent* MoverComp = FindComponentByClass<UMoverComponent>();
	if (!MoverComp)
	{
		return 1.0f;
	}

	const FSpeedUpModifier* SpeedUpMod = MoverComp->FindMovementModifierByType<FSpeedUpModifier>();
	if (!SpeedUpMod)
	{
		return 1.0f;
	}

	return SpeedUpMod->SpeedMultiplier;
}

void APDPawnBase::SetIsAiming(bool bNewAiming)
{
	bIsAiming = bNewAiming;
    
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_SetIsAiming(bNewAiming);
	}
}

void APDPawnBase::Server_SetIsAiming_Implementation(bool bNewAiming)
{
	bIsAiming = bNewAiming;
}

