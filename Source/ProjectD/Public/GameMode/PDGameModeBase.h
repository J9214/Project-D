#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PDGameModeBase.generated.h"



UCLASS()
class PROJECTD_API APDGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	APDGameModeBase();
	
	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	void PlayerDied(AController* Controller);

	void StartOvertime();

	void FinishGame(int32 BestTeamId);

protected:
	void PlayerRespawn(AController* Controller);
	
	void StartRound();

	void OnRoundTick();

	void HandleRoundEnd();
    
	UFUNCTION()
	void OnPlayerOutOfHealth(AController* VictimController, AActor* DamageCauser);

	FTimerHandle RoundTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
	int32 RoundDurationSec;
};
