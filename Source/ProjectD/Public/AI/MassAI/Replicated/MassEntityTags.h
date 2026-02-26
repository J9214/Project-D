#pragma once

#include "MassEntityElementTypes.h"
#include "MassEntityTags.generated.h"

USTRUCT()
struct PROJECTD_API FMassEntityDyingTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct PROJECTD_API FMassEntityPendingRemovalTag : public FMassTag
{
	GENERATED_BODY()
};