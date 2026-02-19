#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TA_ThrowableTrajectory.generated.h"

class USplineComponent;
class UDataAsset_Throwable;
class APDPawnBase;
UCLASS()
class PROJECTD_API ATA_ThrowableTrajectory : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
	ATA_ThrowableTrajectory();

	void SetThrowableData(const UDataAsset_Throwable* InData);

	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ConfirmTargetingAndContinue() override;
	virtual void CancelTargeting() override;
	
	FGameplayAbilityTargetDataHandle MakeTargetData() const;
	
private:
	FVector ComputeStartLocation() const;
	FVector ComputeThrowDirection() const;
	
	void UpdatePreview();
	void ClearPreview();
	
private:
	UPROPERTY()
	TObjectPtr<const UDataAsset_Throwable> ThrowableData;

	UPROPERTY()
	TObjectPtr<APDPawnBase> SourcePawn;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> PreviewSpline;
	
	UPROPERTY()
	FName HandSocketName = TEXT("ThrowableHand");
};
