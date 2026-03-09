#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassBoidsFragment.h"
#include "DroneExplosionFragment.h"
#include "MassBoidsTrait.generated.h"


UCLASS()
class PROJECTD_API UMassBoidsTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Boids")
	FMassBoidsFragment BoidsSettings;

	UPROPERTY(EditAnywhere, Category = "Explosion")
	FDroneExplosionFragment ExplosionSettings;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
