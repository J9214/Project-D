#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PDThrowableItemBase.generated.h"

class UStaticMeshComponent;
class UDataAsset_Throwable;

UCLASS()
class PROJECTD_API APDThrowableItemBase : public AActor
{
	GENERATED_BODY()
	
public:
	APDThrowableItemBase();

	FORCEINLINE UDataAsset_Throwable* GetThrowableData() const { return ThrowableData; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Throwable")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwable")
	TObjectPtr<UDataAsset_Throwable> ThrowableData;
};
