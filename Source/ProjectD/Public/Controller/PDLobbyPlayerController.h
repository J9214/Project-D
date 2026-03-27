// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PDLobbyPlayerController.generated.h"

class UUserWidget;
struct FPDCharacterCustomInfo;
/**
 * 
 */

UCLASS()
class PROJECTD_API APDLobbyPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    void ConnectToDedicatedServer();

    UFUNCTION(BlueprintCallable)
    void RequestTravelToLobby10();

    UFUNCTION(Client, Reliable)
    void Client_RequestCharacterCustomInfo();

    UFUNCTION(Client, Reliable)
    void Client_RequestDisplayName();

    UFUNCTION(Server, Reliable)
    void Server_SubmitDisplayName(const FString& Name);

    UFUNCTION(Server, Reliable)
    void Server_SubmitCharacterCustomInfo(const FPDCharacterCustomInfo& CharacterInfo);

    UFUNCTION(Server, Reliable)
    void Server_RequestTravelToLobby10();
protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess))
    TSubclassOf<UUserWidget> UIWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess))
    TObjectPtr<UUserWidget> UIWidgetInstance;

    TSharedPtr<FOnlineSessionSearch> SessionSearch;
    int32 CurrentSessionIndex = 0;
    int32 MyTeamSize = 1;
    FString PendingDediUrl;
    FDelegateHandle DestroyHandleForDedi;

    UFUNCTION(Client, Reliable)
    void Client_BeginTravelToDedi(const FString& DediUrl);

    UFUNCTION()
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

    void TryJoinNextAvailableSession();

    void OnFindSessionsComplete(bool bWasSuccessful);

};
