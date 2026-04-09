#include "Skills/Terra/Terra_Skill2A_ShieldPush.h"

#include "TP_Character.h"
#include "CombatComponent.h"

void UTerra_Skill2A_ShieldPush::HitStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (!C->CombatComp) return;

	// 전방 광역(부채꼴) 1회 판정
	C->CombatComp->ConfigureAOEForwardHit(
		C->Skill2A_Damage * C->AttackMultiplier,
		C->Skill2A_Radius,
		C->Skill2A_ForwardOffset,
		C->Skill2A_HalfAngleDeg
	);

	C->CombatComp->BeginHitWindow_OneShot();
}