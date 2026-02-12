#include "Chaos/AttributeSet/PDDestructibleAttributeSet.h"

#include "Net/UnrealNetwork.h"

#include "GameplayEffectExtension.h"

const float UPDDestructibleAttributeSet::MIN_DURABILITY = 0.01f;

UPDDestructibleAttributeSet::UPDDestructibleAttributeSet()
{
	InitMaxDurability(50.0f);
	InitDurability(50.0f);
}

void UPDDestructibleAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetDurabilityAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.0f, GetMaxDurability());
	}
}

void UPDDestructibleAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
}

void UPDDestructibleAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;

	if (Attribute == GetDamagedAttribute())
	{
		float LocalDamage = GetDamaged();
		SetDamaged(0.0f);

		if (LocalDamage > 0.0f)
		{
			float OldDurability = GetDurability();
			float NewDurability = FMath::Clamp<float>(OldDurability - LocalDamage, 0.0f, GetMaxDurability());

			if (NewDurability <= 0.0f && OldDurability > 0.0f)
			{
				AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
				FVector HitLocation = FVector::ZeroVector;
				if (Data.EffectSpec.GetContext().GetHitResult())
				{
					HitLocation = Data.EffectSpec.GetContext().GetHitResult()->Location;
				}
				SetDurability(MIN_DURABILITY);

				if (OutOfDurabilityChanged.IsBound())
				{
					OutOfDurabilityChanged.Broadcast(Instigator, HitLocation);
				}
			}
		}
	}
}

void UPDDestructibleAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPDDestructibleAttributeSet, MaxDurability, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPDDestructibleAttributeSet, Durability, COND_None, REPNOTIFY_Always);
}

void UPDDestructibleAttributeSet::OnRep_MaxDurability(const FGameplayAttributeData& OldMaxDurability)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPDDestructibleAttributeSet, MaxDurability, OldMaxDurability);
}

void UPDDestructibleAttributeSet::OnRep_Durability(const FGameplayAttributeData& OldDurability)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPDDestructibleAttributeSet, Durability, OldDurability);
}
