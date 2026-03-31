#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Interface/PDTeamInterface.h"
#include "PDPawnBase.generated.h"

class UWeaponManageComponent;
class UWeaponStateComponent;
class USkillManageComponent;
class UMovementBridgeComponent;
class UDataAsset_InputConfig;
class UDataAsset_StartUpBase;
class USkeletalMeshComponent;
class UCameraComponent;
class USpringArmComponent;
class APDWeaponBase;
class AGoalPost;
class UGameplayEffect;
class UCapsuleComponent;
class UMoverComponent;
class APDCarriableObjectBase;
class UInteractionComponent;
class UPDPlayerUIComponent;
class UWidgetComponent;
class UAnimMontage;
class UNiagaraSystem;
class USoundAttenuation;
class USoundBase;
struct FInputActionValue;
struct FOnAttributeChangeData;
struct FPDCharacterCustomInfo;

UCLASS()
class PROJECTD_API APDPawnBase : public APawn, public IAbilitySystemInterface, public IPDTeamInterface
{
	GENERATED_BODY()

public:
	APDPawnBase();
	
	UFUNCTION(BlueprintPure)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	virtual ETeamType GetTeamID() const override;

	FORCEINLINE UWeaponManageComponent* GetWeaponManageComponent() const { return WeaponManageComponent; }
	FORCEINLINE UWeaponStateComponent* GetWeaponStateComponent() const { return WeaponStateComponent; }
	FORCEINLINE USkillManageComponent* GetSkillManageComponent() const { return SkillManageComponent; }
	FORCEINLINE UMovementBridgeComponent* GetMovementBridgeComponent() const { return MovementBridgeComponent; }
	FORCEINLINE UMoverComponent* GetMoverComponent() const { return MoverComponent; }
	FORCEINLINE UPDPlayerUIComponent* GetUIComponent() const { return UIComponent; }
	FORCEINLINE UWidgetComponent* GetWidgetComponent() const { return WidgetComponent; }

	USkeletalMeshComponent* GetSkeletalMeshComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Placement")
	bool TryConsumePlacementFirstPersonToggleInput();

	UFUNCTION(BlueprintCallable, Category = "Custom")
	void ApplyCustomizationFromPlayerState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Custom")
	void BP_ApplyCharacterCustomization(const FPDCharacterCustomInfo& CharacterCustomInfo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Death")
	void BP_OnDeathFirstPersonReset();
	
	UFUNCTION(Client, Unreliable)
	void ClientDrawFireDebug(const FVector& Start, const FVector& End, bool bHit, const FVector& HitPoint);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;

	void InitAbilityActorInfo();
	void InitAttributeSet();
	void BindAttributeChangeDelegates();
	void BindCustomizationSyncFromPlayerState();
	void UnbindCustomizationSyncFromPlayerState();
	void ApplyCharacterCustomization(const FPDCharacterCustomInfo& CharacterCustomInfo);
	void HandleReplicatedCharacterCustomInfoChanged(const FPDCharacterCustomInfo& NewCharacterCustomInfo);
	
private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	void Input_AbilityInputReleased(FGameplayTag InputTag);
	bool IsPlacementModeActive() const;
	bool ShouldBlockFirstPersonToggleInput() const;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWeaponManageComponent> WeaponManageComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UWeaponStateComponent> WeaponStateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkillManageComponent> SkillManageComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UMovementBridgeComponent> MovementBridgeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData")
	TObjectPtr<UDataAsset_InputConfig> InputConfigDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterData")
	TSoftObjectPtr<UDataAsset_StartUpBase> CharacterStartUpData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mover")
	TObjectPtr<UMoverComponent> MoverComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mover")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UPDPlayerUIComponent> UIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	TWeakObjectPtr<class APDPlayerState> BoundCustomizationPlayerState;
	FDelegateHandle CharacterCustomInfoChangedHandle;
	
#pragma region FirstPerson

protected:
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

	UPROPERTY(BlueprintReadOnly, Category="Anim")
	bool bIsFirstPerson = false;

	UPROPERTY(BlueprintReadOnly, Category="Anim")
	bool bAimHoldDown = false;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CachedCamera = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> CachedSpringArm = nullptr;

	UPROPERTY(Transient)
	float SavedArmLength = 0.f;

	UPROPERTY(Transient)
	bool bSavedDoCollisionTest = true;

	UPROPERTY(EditDefaultsOnly, Category="Camera|FirstPerson")
	float FirstPersonInterpSpeed = 18.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|FirstPerson")
	float FirstPersonFOV = 85.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|FirstPerson")
	float ThirdPersonFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category="Camera|FirstPerson")
	FVector FirstPersonOffsetLocal = FVector::ZeroVector;

	double FirstPersonToggleSuppressUntilTime = -1.0;

protected:
	UFUNCTION(BlueprintCallable)
	void OnAimHoldStarted(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable)
	void OnAimHoldEnded(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable)
	void OnFirstPersonToggle(const FInputActionValue& Value);

	void EnterFirstPerson();
	void ExitFirstPerson();
	void ForceExitFirstPersonOnDeath();
	void UpdateFirstPersonCamera(float DeltaSeconds);

	APDWeaponBase* GetEquippedWeapon() const;
	
#pragma endregion FirstPerson
	
#pragma region Ball
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;

	UFUNCTION(BlueprintCallable)
	void TryInteract();

	UFUNCTION(Server, Reliable)
	void Server_PickUpObject(APDCarriableObjectBase* Object);

	UFUNCTION(Server, Reliable)
	void Server_DropObject(const FVector& InCamDirecion);
	FVector CalcCamDirection();

	void Server_ForceClearCarriedBall();

	APDCarriableObjectBase* GetCarriedObject() const { return CarriedObject.Get(); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName BallSocketName = FName("BallSocket");

protected:
	UPROPERTY(Replicated)
	TWeakObjectPtr<APDCarriableObjectBase> CarriedObject = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GE_HoldingBall;
	void ApplyHoldingBallEffect(); 
	void RemoveHoldingBallEffect(); 

	UFUNCTION(Server, Reliable)
	void Server_TryInteract(AActor* Target);

	AActor* FindInteractTarget() const;

#pragma endregion Ball

#pragma region Respawn

protected:
	virtual void OnDeathTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	virtual void HandleDeathState(bool bIsDead);

	void CacheMeshDeathState();
	void SetDeathMontageEnabled(bool bEnable);
	void HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void SpawnDeathVFX() const;
	void SpawnDeathSound() const;
	void SetDeathVisualHidden(bool bShouldHide) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Montage")
	TObjectPtr<UAnimMontage> DeathMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Montage")
	float DeathMontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Montage")
	float DeathMontageStopBlendOutTime = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|VFX")
	TObjectPtr<UNiagaraSystem> DeathVFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|VFX")
	FVector DeathVFXLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|VFX")
	FVector DeathVFXScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Sound")
	TObjectPtr<USoundBase> DeathSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Sound")
	TObjectPtr<USoundAttenuation> DeathSoundAttenuation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Sound")
	FVector DeathSoundLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Sound")
	float DeathSoundVolumeMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Sound")
	float DeathSoundPitchMultiplier = 1.0f;

	bool bMeshDeathStateCached = false;
	bool bCachedMeshOwnerNoSee = false;
	bool bRootCollisionStateCached = false;
	FName CachedRootCollisionProfileName = NAME_None;
	TEnumAsByte<ECollisionEnabled::Type> CachedRootCollisionEnabled = ECollisionEnabled::NoCollision;

#pragma endregion Respawn

#pragma region mover

public:
	UFUNCTION(BlueprintCallable, Category = "Mover")
	FVector GetDirectionByMoveInput(const FVector& FallbackForward) const;
	
	UFUNCTION(BlueprintCallable, Category = "Mover")
	void CancelMovementGA();
	
	UFUNCTION(BlueprintPure, Category="Movement")
	float GetActiveSpeedUpMultiplier() const;
	
#pragma endregion mover
	
#pragma region Animation
	
protected:
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Anim")
	bool bIsAiming = false;
	
	UFUNCTION(BlueprintCallable , Category = "Anim")
	void SetIsAiming(bool bNewAiming);
	
	UFUNCTION(Server, Reliable)
	void Server_SetIsAiming(bool bNewAiming);

#pragma endregion Animation

};
