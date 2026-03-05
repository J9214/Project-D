#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
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
struct FInputActionValue;
struct FOnAttributeChangeData;

UCLASS()
class PROJECTD_API APDPawnBase : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APDPawnBase();
	
	UFUNCTION(BlueprintPure)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	FORCEINLINE UWeaponManageComponent* GetWeaponManageComponent() const { return WeaponManageComponent; }
	FORCEINLINE UWeaponStateComponent* GetWeaponStateComponent() const { return WeaponStateComponent; }
	FORCEINLINE USkillManageComponent* GetSkillManageComponent() const { return SkillManageComponent; }
	FORCEINLINE UMovementBridgeComponent* GetMovementBridgeComponent() const { return MovementBridgeComponent; }
	FORCEINLINE UMoverComponent* GetMoverComponent() const { return MoverComponent; }

	USkeletalMeshComponent* GetSkeletalMeshComponent() const;
	
	UFUNCTION(Client, Unreliable)
	void ClientDrawFireDebug(const FVector& Start, const FVector& End, bool bHit, const FVector& HitPoint);

protected:
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void InitAbilityActorInfo();
	void InitAttributeSet();
	void BindAttributeChangeDelegates();
	
private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);
	
	void Input_AbilityInputPressed(FGameplayTag InputTag);
	void Input_AbilityInputReleased(FGameplayTag InputTag);

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

protected:
	UFUNCTION(BlueprintCallable)
	void OnAimHoldStarted(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable)
	void OnAimHoldEnded(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable)
	void OnFirstPersonToggle(const FInputActionValue& Value);

	void EnterFirstPerson();
	void ExitFirstPerson();
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

#pragma endregion Respawn

#pragma region mover

public:
	UFUNCTION(BlueprintCallable, Category = "Mover")
	FVector GetDirectionByMoveInput(const FVector& FallbackForward) const;
	
	UFUNCTION(BlueprintCallable, Category = "Mover")
	void CancelMovementGA();
	
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
