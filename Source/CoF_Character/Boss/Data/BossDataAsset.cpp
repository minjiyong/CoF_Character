#include "BossDataAsset.h"

UBossDataAsset::UBossDataAsset()
    : BossName(TEXT("DefaultBoss"))
    , MaxHP(1000.f)
    , WalkSpeed(300.f)
    , DetectRange(1200.f)
    , AttackRange(200.f)
    , AttackCooldown(2.f)
    , AttackDamage(10.f)
    , IntroMontage(nullptr)
    , AttackMontage(nullptr)
    , HitReactMontage(nullptr)
    , DeadMontage(nullptr)
{
}