#include "Gimmick/PDDirectlyInteractGimmickBase.h"

bool APDDirectlyInteractGimmickBase::IsCanInteract(AActor* Interactor)
{
	return IsValid(Interactor) && bIsCanInteract;
}
