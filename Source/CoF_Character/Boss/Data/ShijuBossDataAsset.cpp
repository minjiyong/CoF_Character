#include "ShijuBossDataAsset.h"

UShijuBossDataAsset::UShijuBossDataAsset()
{
    BossName = TEXT("Shiju");
    MaxHP = 1500.f;
    WalkSpeed = 350.f;
    DetectRange = 2500.f;
    AttackRange = 1800.f;
    AttackCooldown = 1.5f;
    AttackDamage = 10.f;

    DesiredCombatRange = 1300.f;
    TooCloseRange = 700.f;

    BasicArrowCooldown = 1.5f;
    PiercingShotCooldown = 5.0f;
    PiercingChargeTime = 0.8f;

    BasicArrowDamage = 10.f;
    PiercingArrowDamage = 25.f;

    ArrowSpeed = 2200.f;
    PiercingProjectileSpeed = 4200.f;

    QCooldown = 8.0f;
    QCastDelay = 0.10f;
    QRadius = 350.f;
    QDamage = 20.f;
    QMaxRange = 2000.f;
    QArcHeight = 450.f;
    QBurnDuration = 3.0f;
    QBurnTickInterval = 0.5f;

    RCooldown = 12.0f;
    RArrowCount = 8;
    RFireInterval = 0.14f;
    RStartDelay = 0.20f;
    RArrowDamage = 12.0f;
    RProjectileSpeed = 4200.f;

    TimeMarkDuration = 3.0f;
    MaxTimeMarkStack = 4;
    BellPassiveCooldown = 10.0f;
    BellTriggerSound = nullptr;

    Phase2StartHPRatio = 0.7f;
    Phase3StartHPRatio = 0.35f;
    Phase3MaxTimeMarkStack = 3;
    Phase3RemovePillarTag = TEXT("ShijuPhase3Remove");

    BasicArrowMontage = nullptr;
    PiercingShotMontage = nullptr;
    QMontage = nullptr;
    RMontage = nullptr;

    ArrowProjectileClass = nullptr;
    BasicArrowProjectileClass = nullptr;
    PiercingProjectileClass = nullptr;
    QProjectileClass = nullptr;
    RProjectileClass = nullptr;

    QAreaActorClass = nullptr;
}