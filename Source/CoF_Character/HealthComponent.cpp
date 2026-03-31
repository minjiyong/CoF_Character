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

	// 1) 보호막 먼저 깎기
	if (Shield > 0.f)
	{
		const float Absorb = FMath::Min(Shield, DamageAmount);
		Shield -= Absorb;
		DamageAmount -= Absorb;
	}

	// 2) 남은 데미지만 HP에 적용
	if (DamageAmount > 0.f)
	{
		CurrentHp = FMath::Clamp(CurrentHp - DamageAmount, 0.f, MaxHp);
	}

	if (AActor* Owner = GetOwner())
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Damage applied: Shield %.1f, HP %.1f"),
			*Owner->GetName(), Shield, CurrentHp);
	}
}


void UHealthComponent::AddShield(float Bonus)
{
	if (Bonus <= 0.f) return;

	Shield = FMath::Max(0.f, Shield + Bonus);
}

void UHealthComponent::RemoveShield(float Bonus)
{
	if (Bonus <= 0.f) return;

	// 남아있는 보호막에서 Bonus만큼 제거 (남은게 더 적으면 0으로)
	Shield = FMath::Max(0.f, Shield - Bonus);
}