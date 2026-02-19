#pragma once

#include "CoreMinimal.h"
#include "Gimmick/PDGimmickBase.h"
#include "PDDirectlyInteractGimmickBase.generated.h"

UCLASS(Abstract)
class PROJECTD_API APDDirectlyInteractGimmickBase : public APDGimmickBase
{
	GENERATED_BODY()
	
public:
	virtual bool IsCanInteract(AActor* Interactor);
};
