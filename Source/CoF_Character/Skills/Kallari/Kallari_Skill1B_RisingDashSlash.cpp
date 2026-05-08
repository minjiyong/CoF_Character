#include "Skills/Kallari/Kallari_Skill1B_RisingDashSlash.h"

#include "CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TP_Character.h"

void UKallari_Skill1B_RisingDashSlash::HitStart()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;
    if (!C->CombatComp) return;

    C->CombatComp->ConfigureDashHit(
        C->Skill1B_Damage * C->AttackMultiplier,
        C->Skill1B_RisingDuration,
        C->Skill1B_RisingHitRadius
    );

    C->CombatComp->BeginHitWindow_OneShot();
}

void UKallari_Skill1B_RisingDashSlash::DashStart()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;

    UCharacterMovementComponent* Move = C->GetCharacterMovement();
    if (!Move) return;

    // Terra dash 로직 최대 재사용, 방향만 Z축 상승으로 변경
    Move->StopMovementImmediately();
    Move->SetMovementMode(MOVE_Flying);
    Move->GroundFriction = 0.f;
    Move->BrakingFrictionFactor = 0.f;
    Move->BrakingDecelerationFlying = 0.f;

    const float Speed =
        (C->Skill1B_RisingDuration > KINDA_SMALL_NUMBER)
        ? (C->Skill1B_RisingDistance / C->Skill1B_RisingDuration)
        : 0.f;

    Move->Velocity = FVector::UpVector * Speed;
}

//상승 dash 종료 시 공중 낙하 상태로 복귀시킴, Terra는 바로 걷기라서..
void UKallari_Skill1B_RisingDashSlash::DashEnd()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;

    UCharacterMovementComponent* MoveComp = C->GetCharacterMovement();
    if (!MoveComp || !bDashMoving) return;

    bDashMoving = false;

    // 복구
    MoveComp->GroundFriction = SavedGroundFriction;
    MoveComp->BrakingFrictionFactor = SavedBrakingFrictionFactor;
    MoveComp->BrakingDecelerationWalking = SavedBrakingDecelerationWalking;
    MoveComp->BrakingDecelerationFlying = SavedBrakingDecelerationFlying;
    MoveComp->bOrientRotationToMovement = bSavedOrientRotationToMovement;
    C->bUseControllerRotationYaw = bSavedUseControllerRotationYaw;

    // 상승 돌진은 공중에서 끝나므로 걷기 모드가 아니라 낙하 모드로 복귀
    MoveComp->Velocity = FVector::ZeroVector;
    MoveComp->SetMovementMode(MOVE_Falling);
}