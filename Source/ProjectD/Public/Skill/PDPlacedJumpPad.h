#pragma once

#include "CoreMinimal.h"
#include "Skill/PDDamageableSkillActor.h"
#include "PDPlacedJumpPad.generated.h"

class UPDJumpPadDataAsset;
class USphereComponent;
class UPrimitiveComponent;
class UAbilitySystemComponent;
struct FActiveGameplayEffectHandle;
struct FGameplayEffectSpec;

UCLASS()
class PROJECTD_API APDPlacedJumpPad : public APDDamageableSkillActor
{
	GENERATED_BODY()

public:
	APDPlacedJumpPad();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void OnEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle) override;

	UFUNCTION()
	void HandlePadOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void LaunchActorByPad(AActor* Interactor);
	float ExtractDamageFromSpec(const FGameplayEffectSpec& Spec) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="JumpPad|Component")
	TObjectPtr<USphereComponent> TriggerSphere = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="JumpPad")
	TObjectPtr<UPDJumpPadDataAsset> JumpPadDataAsset = nullptr;
};
