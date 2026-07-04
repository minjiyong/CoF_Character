#include "HealthComponent.h"

#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BroadcastHealthStatus()
{
	OnHealthStatusChanged.Broadcast(
		CurrentHp,
		MaxHp,
		Shield
	);
}

void UHealthComponent::ResetHp()
{
	CurrentHp = MaxHp;

	OnHpChanged.Broadcast(
		CurrentHp,
		0.f
	);

	BroadcastHealthStatus();
}

void UHealthComponent::ApplyDamage_Local(float DamageAmount)
{
	if (DamageAmount <= 0.f || IsDead())
	{
		return;
	}

	const float PreviousHp = CurrentHp;

	// 1) 보호막 먼저 깎기
	if (Shield > 0.f)
	{
		const float Absorb =
			FMath::Min(Shield, DamageAmount);

		Shield -= Absorb;
		DamageAmount -= Absorb;
	}

	// 2) 남은 데미지만 HP에 적용
	if (DamageAmount > 0.f)
	{
		CurrentHp = FMath::Clamp(
			CurrentHp - DamageAmount,
			0.f,
			MaxHp
		);
	}

	// 실제 HP가 변경됐을 때 기존 이벤트도 유지한다.
	if (!FMath::IsNearlyEqual(PreviousHp, CurrentHp))
	{
		OnHpChanged.Broadcast(
			CurrentHp,
			CurrentHp - PreviousHp
		);
	}

	// 체력 또는 보호막 변경 내용을 HUD에 전달한다.
	BroadcastHealthStatus();

	if (AActor* Owner = GetOwner())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[%s] Damage applied: Shield %.1f, HP %.1f"),
			*Owner->GetName(),
			Shield,
			CurrentHp
		);
	}
}

void UHealthComponent::AddShield(float Bonus)
{
	if (Bonus <= 0.f)
	{
		return;
	}

	Shield = FMath::Max(
		0.f,
		Shield + Bonus
	);

	BroadcastHealthStatus();
}

void UHealthComponent::RemoveShield(float Bonus)
{
	if (Bonus <= 0.f)
	{
		return;
	}

	// 남아있는 보호막에서 Bonus만큼 제거
	// 남은 보호막이 더 적으면 0으로 만든다.
	Shield = FMath::Max(
		0.f,
		Shield - Bonus
	);

	BroadcastHealthStatus();
}