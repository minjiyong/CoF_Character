#include "Skills/Terra/Terra_Skill1A_Dash.h"

#include "TP_Character.h"
#include "CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTerra_Skill1A_Dash::HitStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (!C->CombatComp) return;

	// 마지막 radius, 나중에 캐릭터데이터에 추가해서 캐싱하기
	C->CombatComp->ConfigureDashHit(C->Skill1A_Damage * C->AttackMultiplier, C->Skill1A_DashDuration, 80.f);
	C->CombatComp->BeginHitWindow_OneShot();
}

void UTerra_Skill1A_Dash::HitEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (!C->CombatComp) return;

	C->CombatComp->EndHitWindow();
}

void UTerra_Skill1A_Dash::DashStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	UCharacterMovementComponent* MoveComp = C->GetCharacterMovement();
	if (!MoveComp || C->bSkillDashMoving) return;

	C->bSkillDashMoving = true;

	// 기존 저장값
	C->SavedGroundFriction = MoveComp->GroundFriction;
	C->SavedBrakingFrictionFactor = MoveComp->BrakingFrictionFactor;
	C->SavedBrakingDecelerationWalking = MoveComp->BrakingDecelerationWalking;
	C->SavedBrakingDecelerationFlying = MoveComp->BrakingDecelerationFlying;
	C->bSavedOrientRotationToMovement = MoveComp->bOrientRotationToMovement;
	C->bSavedUseControllerRotationYaw = C->bUseControllerRotationYaw;

	// 대쉬 동안 마찰/감속 제거
	MoveComp->GroundFriction = 0.f;
	MoveComp->BrakingFrictionFactor = 0.f;
	MoveComp->BrakingDecelerationWalking = 0.f;
	MoveComp->BrakingDecelerationFlying = 0.f;

	MoveComp->bOrientRotationToMovement = false;
	C->bUseControllerRotationYaw = false;

	// 비행 모드로 바꿔서 바닥 마찰/충돌 영향 최소화
	MoveComp->SetMovementMode(MOVE_Flying);

	const FVector Dir = C->GetActorForwardVector().GetSafeNormal2D();
	const float Speed = (C->Skill1A_DashDuration > 0.f) ? (C->Skill1A_DashDistance / C->Skill1A_DashDuration) : 0.f;

	MoveComp->Velocity = Dir * Speed;
}

void UTerra_Skill1A_Dash::DashEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	UCharacterMovementComponent* MoveComp = C->GetCharacterMovement();
	if (!MoveComp || !C->bSkillDashMoving) return;

	C->bSkillDashMoving = false;

	MoveComp->StopMovementImmediately();
	MoveComp->SetMovementMode(MOVE_Walking);

	// 원복
	MoveComp->GroundFriction = C->SavedGroundFriction;
	MoveComp->BrakingFrictionFactor = C->SavedBrakingFrictionFactor;
	MoveComp->BrakingDecelerationWalking = C->SavedBrakingDecelerationWalking;
	MoveComp->BrakingDecelerationFlying = C->SavedBrakingDecelerationFlying;
	MoveComp->bOrientRotationToMovement = C->bSavedOrientRotationToMovement;
	C->bUseControllerRotationYaw = C->bSavedUseControllerRotationYaw;
}