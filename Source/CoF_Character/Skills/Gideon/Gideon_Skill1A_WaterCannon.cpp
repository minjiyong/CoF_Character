#include "Skills/Gideon/Gideon_Skill1A_WaterCannon.h"

#include "CombatComponent.h"
#include "TP_Character.h"

void UGideon_Skill1A_WaterCannon::HitStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;

	C->CombatComp->ConfigureTraceHit(
		C->Skill1A_Damage * C->AttackMultiplier,
		C->Skill1A_Range,
		C->Skill1A_StartSocket
	);

	C->CombatComp->BeginHitWindow_OneShot();
}

void UGideon_Skill1A_WaterCannon::HitEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;

	C->CombatComp->EndHitWindow();
}