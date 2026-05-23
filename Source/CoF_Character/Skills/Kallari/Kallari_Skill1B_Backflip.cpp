#include "Skills/Kallari/Kallari_Skill1B_Backflip.h"

#include "TP_Character.h"

void UKallari_Skill1B_Backflip::DashStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	// 뒤 방향으로만, 실제 목표 거리만큼 이동
	const FVector BackOffset =
		-C->GetActorForwardVector().GetSafeNormal2D() * C->Skill1B_BackwardDistance;

	BeginDashMoveToOffset(
		BackOffset,
		C->Skill1B_BackflipDuration,
		MOVE_Falling,
		MOVE_Falling,
		true
	);
}

void UKallari_Skill1B_Backflip::DashEnd()
{
	// 백플립 끝 지점까지 정확히 간 뒤 Falling 유지
	FinishDashMove(true);
}