#include "Skills/Kallari/Kallari_Skill1A_DashSlash.h"

#include "CombatComponent.h"
#include "TP_Character.h"

void UKallari_Skill1A_DashSlash::HitStart()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;
    if (!C->CombatComp) return;

    C->CombatComp->ConfigureDashHit(
        C->Skill1A_Damage * C->AttackMultiplier,
        C->Skill1A_DashDuration,
        C->Skill1A_HitRadius
    );

    C->CombatComp->BeginHitWindow_OneShot();
}

void UKallari_Skill1A_DashSlash::DashStart()
{
    // Terra 로직 재사용
    UTerra_Skill1A_Dash::DashStart();
}