#include "Gimmick/PDOverlapGimmickBase.h"

#include "Components/ShapeComponent.h"

void APDOverlapGimmickBase::BeginPlay()
{
	Super::BeginPlay();

	if (Shape.IsValid() && !Shape->OnComponentBeginOverlap.IsAlreadyBound(this, &APDOverlapGimmickBase::OnOverlapGimmick))
	{
		Shape->OnComponentBeginOverlap.AddDynamic(this, &APDOverlapGimmickBase::OnOverlapGimmick);
	}
}

void APDOverlapGimmickBase::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (Shape.IsValid() && Shape->OnComponentBeginOverlap.IsAlreadyBound(this, &APDOverlapGimmickBase::OnOverlapGimmick))
	{
		Shape->OnComponentBeginOverlap.RemoveDynamic(this, &APDOverlapGimmickBase::OnOverlapGimmick);
	}

	Super::EndPlay(EndPlayReason);
}

void APDOverlapGimmickBase::OnOverlapGimmick(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	Execute_OnInteract(this, OtherActor);
}
