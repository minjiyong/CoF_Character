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
