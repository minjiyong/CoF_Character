#include "Skills/Terra/Terra_Skill1B_SlamAOE.h"

#include "TP_Character.h"
#include "CombatComponent.h"

void UTerra_Skill1B_SlamAOE::ApplyAOE()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (!C->CombatComp) return;

	// 여기서 스킬1 내려찍기(검) 의 광역 판정 1회 실행
	C->CombatComp->ConfigureAOEHit(C->Skill1B_Damage * C->AttackMultiplier, C->Skill1B_Radius);
	C->CombatComp->BeginHitWindow_OneShot();
}