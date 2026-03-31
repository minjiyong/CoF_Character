#include "HealthComponent.h"
#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::ResetHp()
{
	CurrentHp = MaxHp;
	OnHpChanged.Broadcast(CurrentHp, 0.f);
}

void UHealthComponent::ApplyDamage_Local(float DamageAmount)
{
	if (DamageAmount <= 0.f || IsDead()) return;

	const float Old = CurrentHp;
	CurrentHp = FMath::Clamp(CurrentHp - DamageAmount, 0.f, MaxHp);

	const float Delta = CurrentHp - Old;
	OnHpChanged.Broadcast(CurrentHp, Delta);

	if (AActor* Owner = GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HP] %s HP=%.1f (Delta=%.1f)"),
			*Owner->GetName(), CurrentHp, Delta);
	}
}


void UHealthComponent::AddMaxHpBonus(float Bonus, bool bHealAlso)
{
	MaxHp += Bonus;
	if (bHealAlso)
		CurrentHp = FMath::Clamp(CurrentHp + Bonus, 0.f, MaxHp);
	else
		CurrentHp = FMath::Clamp(CurrentHp, 0.f, MaxHp);
}

void UHealthComponent::RemoveMaxHpBonus(float Bonus)
{
	MaxHp = FMath::Max(1.f, MaxHp - Bonus);
	CurrentHp = FMath::Clamp(CurrentHp, 0.f, MaxHp);
}