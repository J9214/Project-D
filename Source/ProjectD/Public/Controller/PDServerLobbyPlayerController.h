// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameMode/PDLobbyGameMode.h"
#include "PDServerLobbyPlayerController.generated.h"

class UUserWidget;
class UServerLobby;

UCLASS()
class PROJECTD_API APDServerLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void OnRep_PlayerState() override;

    UFUNCTION(Client, Reliable)
    void Client_UpdateLobbyTeamInfos(const TArray<FTeamInfo>& InTeamInfos, float InMatchStartServerTimeSec);

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI)
    TSubclassOf<UUserWidget> UIWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI)
    TObjectPtr<UUserWidget> UIWidgetInstance;
    
private:
    void RefreshLobbyWidget();
    ETeamType GetLocalTeamID() const;

    TArray<FTeamInfo> CachedTeamInfos;
    float CachedMatchStartServerTimeSec = 0.0f;
    bool bHasCachedMatchStartServerTime = false;

    UPROPERTY(Transient)
    TObjectPtr<UServerLobby> ServerLobbyWidget;
};
