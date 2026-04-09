#include "Skills/Terra/Terra_Skill2B_Spin.h"

#include "TP_Character.h"
#include "CombatComponent.h"
#include "Animation/AnimInstance.h"

void UTerra_Skill2B_Spin::HitStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (!C->CombatComp) return;

	// Spin 판정 시작
	C->CombatComp->ConfigureSpinHit(
		C->Skill2B_DamagePerTick * C->AttackMultiplier,
		C->Skill2B_Radius,
		C->Skill2B_TickInterval,
		C->Skill2B_Duration
	);

	C->CombatComp->BeginHitWindow_OneShot();
}

void UTerra_Skill2B_Spin::SpinEnd() // 돌기 시간 끝나면 End로
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	// 시전 종료 시간이 아직이면 아무것도 안 함
	if (C->GetWorld()->GetTimeSeconds() < C->Skill2B_EndTime)
		return;

	UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim || !C->Skill2MontageB) return;

	Anim->Montage_SetNextSection(FName(TEXT("Loop")), FName(TEXT("End")), C->Skill2MontageB);
}