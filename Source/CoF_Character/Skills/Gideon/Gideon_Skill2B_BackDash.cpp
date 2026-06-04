#include "Skills/Gideon/Gideon_Skill2B_BackDash.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "TP_Character.h"

void UGideon_Skill2B_BackDash::DashStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	UCharacterMovementComponent* MoveComp = C->GetCharacterMovement();
	if (!MoveComp || bDashMoving) return;

	FVector Backward = -C->GetActorForwardVector();
	Backward.Z = 0.f;
	Backward = Backward.GetSafeNormal();

	if (Backward.IsNearlyZero())
	{
		return;
	}

	const FVector WorldOffset = Backward * C->Skill2B_BackwardDistance;

	// 백대쉬 중에는 바닥에 걸리지 않게 Flying으로 시작하고, 끝나면 Walking 복귀
	BeginDashMoveToOffset(
		WorldOffset,
		C->Skill2B_BackDashDuration,
		MOVE_Flying,
		MOVE_Walking,
		true
	);
}

void UGideon_Skill2B_BackDash::DashEnd()
{
	// 몽타주 notify에서 강제 종료할 수 있게 마무리 호출
	if (!bDashMoving) return;

	FinishDashMove(false);
}