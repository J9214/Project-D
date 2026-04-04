#pragma once

#include "CoreMinimal.h"
#include "Gimmick/PDGimmickBase.h"
#include "PDOverlapGimmickBase.generated.h"

UCLASS(Abstract)
class PROJECTD_API APDOverlapGimmickBase : public APDGimmickBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION()
	void OnOverlapGimmick(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

protected:
	TWeakObjectPtr<UShapeComponent> Shape;
};
