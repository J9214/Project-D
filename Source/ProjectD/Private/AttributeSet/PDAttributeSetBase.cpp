#include "AttributeSet/PDAttributeSetBase.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UPDAttributeSetBase::UPDAttributeSetBase()
{
	InitMaxHealth(200.f);
	InitHealth(200.f);
	InitMoveSpeed(600.f);
}

void UPDAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max<float>(1.0f, NewValue);
		
		const float NewMax = NewValue;
		const float CurrentHealth = GetHealth();

		if (CurrentHealth > NewMax)
		{
			if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
			{
				ASC->SetNumericAttributeBase(GetHealthAttribute(), NewMax);
			}
		}
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max<float>(0.0f, NewValue);
	}
}

void UPDAttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
}

void UPDAttributeSetBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;

	if (Attribute == GetDamagedAttribute())
	{
		float LocalDamage = GetDamaged();
		SetDamaged(0.f);

		if (LocalDamage > 0.f)
		{
			float OldHealth = GetHealth();
			float NewHealth = FMath::Clamp(OldHealth - LocalDamage, 0.f, GetMaxHealth());
			SetHealth(NewHealth);

			if (NewHealth <= 0.0f && OldHealth > 0.0f)
			{
				AActor* TargetActor = Data.Target.GetAvatarActor();
				AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
				AController* TargetController = nullptr;

				if (APawn* TargetPawn = Cast<APawn>(TargetActor))
				{
					TargetController = TargetPawn->GetController();
				}

				if (TargetController && TargetController->HasAuthority())
				{
					if (OnOutOfHealth.IsBound())
					{
						OnOutOfHealth.Broadcast(TargetController, Instigator);
					}
				}
			}
		}
	}
}

void UPDAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UPDAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPDAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPDAttributeSetBase, MoveSpeed, COND_None, REPNOTIFY_Always);
}

void UPDAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPDAttributeSetBase, MaxHealth, OldMaxHealth);
}

void UPDAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPDAttributeSetBase, Health, OldHealth);
}

void UPDAttributeSetBase::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPDAttributeSetBase, MoveSpeed, OldMoveSpeed);
}

