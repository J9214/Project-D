#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PDTeamInterface.h"
#include "PDPlacedSkillActorBase.generated.h"

UCLASS()
class PROJECTD_API APDPlacedSkillActorBase : public AActor, public IPDTeamInterface
{
	GENERATED_BODY()
	
public:
	APDPlacedSkillActorBase();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category="PlacedActor")
	void InitializePlacedActor(float InLifeTime, ETeamType InOwnerTeamID);

	UFUNCTION(BlueprintCallable, Category="PlacedActor")
	void SetOwnerTeamID(ETeamType InOwnerTeamID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="PlacedActor")
	void RequestDestroy(bool bDestroy);
	virtual void RequestDestroy_Implementation(bool bDestroy);

	virtual ETeamType GetTeamID() const override { return OwnerTeamID; }

protected:
	UFUNCTION()
	virtual void HandleLifeExpired();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category="PlacedActor")
	float LifeTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category="PlacedActor")
	bool bAutoDestroyByLifeTime = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category="PlacedActor")
	ETeamType OwnerTeamID = ETeamType::None;

	FTimerHandle LifeTimerHandle;
};