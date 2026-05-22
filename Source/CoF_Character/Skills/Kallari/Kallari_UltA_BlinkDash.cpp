#include "Skills/Kallari/Kallari_UltA_BlinkDash.h"

#include "CombatComponent.h"
#include "TP_Character.h"

void UKallari_UltA_BlinkDash::HitStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;

	C->CombatComp->ConfigureDashHit(
		C->UltA_Damage * C->AttackMultiplier,
		C->UltA_DashDuration,
		C->UltA_HitRadius
	);

	C->CombatComp->BeginHitWindow_OneShot();
}

void UKallari_UltA_BlinkDash::DashStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	const FVector ForwardOffset =
		C->GetActorForwardVector().GetSafeNormal2D() * C->UltA_DashDistance;

	BeginDashMoveToOffset(
		ForwardOffset,
		C->UltA_DashDuration,
		MOVE_Flying,
		MOVE_Walking,
		true
	);
}

void UKallari_UltA_BlinkDash::DashEnd()
{
	// blink dash는 목표 지점까지 정확히 간 뒤 정리
	FinishDashMove(true);
}