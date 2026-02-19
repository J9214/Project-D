#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GA_Throw.generated.h"

class UAbilityTask_WaitTargetData;
class UWeaponManageComponent;
class UDataAsset_Throwable;
class ATA_ThrowableTrajectory;
class APDThrowableItemBase;
struct FPDTargetData_Throwable;

UCLASS()
class PROJECTD_API UGA_Throw : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Throw();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled
	) override;
	
protected:
	UFUNCTION()
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);

	UFUNCTION()
	void OnTargetDataCancelled();
	
private:
	UDataAsset_Throwable* GetEquippedThrowableDA();
	UWeaponManageComponent* GetWMC();
	
	void SpawnProjectile(const FPDTargetData_Throwable& TD, UDataAsset_Throwable* DA);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Throw")
	TSubclassOf<ATA_ThrowableTrajectory> TargetActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Throw")
	FName ThrowHandSocketName = TEXT("ThrowableHand");
	
private:
	UPROPERTY()
	TObjectPtr<ATA_ThrowableTrajectory> CachedTA = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitTargetData> WaitTargetDataTask = nullptr;
};
