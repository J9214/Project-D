#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_Fire.generated.h"

class APDPawnBase;
class APDWeaponBase;
class UAbilityTask_WaitDelay;
struct FGameplayAbilityTargetDataHandle;
struct FHitResult;

UCLASS()
class PROJECTD_API UGA_Fire : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Fire();
	
	void HandleServerReceivedTargetData(
		const FGameplayAbilityTargetDataHandle& Data,
		FGameplayTag ActivationTag,
		FGameplayAbilitySpecHandle Handle,
		FPredictionKey ShotKey
	);
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	
	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	UFUNCTION()
	void OnWaitDelayFinished();

	void FireOneShot();
	void StartFireNow();
	void ScheduleNextShot(float Interval);

	bool GetOwnerPawnWeapon(UAbilitySystemComponent*& OutASC, APDPawnBase*& OutPawn, APDWeaponBase*& OutWeapon);
	FVector CalcLocalAimPoint(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon) const;
	FVector GetChestShotStart(APDPawnBase* OwnerPawn) const;
	FGameplayAbilityTargetDataHandle MakeAimPointTargetData(const FVector& CameraStart, const FVector& AimPoint);
	
	void MuzzleTraceAndApplyGE(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon, const FVector& AimPoint);
	void ApplyWeaponDamageGE(const FHitResult& Hit, const APDWeaponBase* Weapon, float DamageValue);
	void ApplyFireCooldownToOwner(const APDWeaponBase* Weapon);
	
	void PlayLocalFireFX(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon);
	
	void BindServerTargetDataDelegate();
	void UnbindServerTargetDataDelegate();
	void OnServerTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Tag);
	
	void HandleServerReceivedTargetData_Internal(const FGameplayAbilityTargetDataHandle& Data);
	
	void TraceSingleShot(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon, const FVector& AimPoint);
	void TraceMultiBulletShot(APDPawnBase* OwnerPawn, APDWeaponBase* Weapon, const FVector& AimPoint);
	
	TArray<FVector> BuildBulletDirections(
		const FVector& BaseDir,
		int32 BulletCount,
		float SpreadHalfAngleDeg,
		bool bIncludeCenterBullet
	) const;

	float GetDamagePerBullet(const APDWeaponBase* Weapon) const;
	
protected:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> WaitDelayTask = nullptr;
	
	bool bKeepFiring = false;
	
	FDelegateHandle ServerTDDelegateHandle;

	bool bServerDelegateBound = false;
};
