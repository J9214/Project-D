#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/PDPlayerGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_Dash.generated.h"

class UAnimMontage;

UCLASS()
class PROJECTD_API UGA_Dash : public UPDPlayerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Dash();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
public:
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float DashDistance = 600.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float DashDurationMs = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	int32 Priority = 10;
	
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	TObjectPtr<UAnimMontage> DashMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="Dash|Montage")
	TObjectPtr<UAnimMontage> DashMontage_F;

	UPROPERTY(EditDefaultsOnly, Category="Dash|Montage")
	TObjectPtr<UAnimMontage> DashMontage_B;

	UPROPERTY(EditDefaultsOnly, Category="Dash|Montage")
	TObjectPtr<UAnimMontage> DashMontage_L;

	UPROPERTY(EditDefaultsOnly, Category="Dash|Montage")
	TObjectPtr<UAnimMontage> DashMontage_R;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CueTag")
	FGameplayTag ExplosionCueTag;
	
	UAnimMontage* SelectDashMontage(const APawn* Pawn, const FVector& MoveDir) const;
	
	void ExecuteDashCue(const FVector& DashDir) const;
	
	
};
