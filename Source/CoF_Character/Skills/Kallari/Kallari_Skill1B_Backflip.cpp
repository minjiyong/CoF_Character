#include "Skills/Kallari/Kallari_Skill1B_Backflip.h"

#include "TP_Character.h"

void UKallari_Skill1B_Backflip::DashStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	const FVector BackOffset =
		-C->GetActorForwardVector().GetSafeNormal2D() * C->Skill1B_BackwardDistance;

	const FVector UpOffset =
		FVector::UpVector * C->Skill1B_UpwardDistance;

	BeginDashMoveToOffset(
		BackOffset + UpOffset,
		C->Skill1B_BackflipDuration,
		MOVE_Falling,
		MOVE_Falling,
		true
	);
}

void UKallari_Skill1B_Backflip::DashEnd()
{
	// 공중제비 끝 지점까지 정확히 간 뒤 Falling 유지
	FinishDashMove(true);
}