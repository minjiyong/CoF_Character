#include "Skills/Kallari/Kallari_UltA_BlinkDash.h"

#include "CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TP_Character.h"

void UKallari_UltA_BlinkDash::HitStart()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;
    if (!C->CombatComp) return;

    C->CombatComp->ConfigureDashHit(
        C->UltA_Damage * C->AttackMultiplier,
        C->UltA_DashDuration,
        C->UltA_HitRadius
    );

    C->CombatComp->BeginHitWindow_OneShot();
}

void UKallari_UltA_BlinkDash::DashStart()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;

    UCharacterMovementComponent* MoveComp = C->GetCharacterMovement();
    if (!MoveComp || bDashMoving) return;

    bDashMoving = true;

    SavedGroundFriction = MoveComp->GroundFriction;
    SavedBrakingFrictionFactor = MoveComp->BrakingFrictionFactor;
    SavedBrakingDecelerationWalking = MoveComp->BrakingDecelerationWalking;
    SavedBrakingDecelerationFlying = MoveComp->BrakingDecelerationFlying;
    bSavedOrientRotationToMovement = MoveComp->bOrientRotationToMovement;
    bSavedUseControllerRotationYaw = C->bUseControllerRotationYaw;

    MoveComp->bOrientRotationToMovement = false;
    C->bUseControllerRotationYaw = false;

    MoveComp->StopMovementImmediately();
    MoveComp->SetMovementMode(MOVE_Flying);
    MoveComp->GroundFriction = 0.f;
    MoveComp->BrakingFrictionFactor = 0.f;
    MoveComp->BrakingDecelerationFlying = 0.f;

    const float Duration =
        (C->UltA_DashDuration > KINDA_SMALL_NUMBER)
        ? C->UltA_DashDuration
        : 0.01f;

    const float Speed = C->UltA_DashDistance / Duration;

    MoveComp->Velocity = C->GetActorForwardVector() * Speed;
    MoveComp->UpdateComponentVelocity();
}

void UKallari_UltA_BlinkDash::DashEnd()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;

    UCharacterMovementComponent* MoveComp = C->GetCharacterMovement();
    if (!MoveComp || !bDashMoving) return;

    bDashMoving = false;

    MoveComp->GroundFriction = SavedGroundFriction;
    MoveComp->BrakingFrictionFactor = SavedBrakingFrictionFactor;
    MoveComp->BrakingDecelerationWalking = SavedBrakingDecelerationWalking;
    MoveComp->BrakingDecelerationFlying = SavedBrakingDecelerationFlying;
    MoveComp->bOrientRotationToMovement = bSavedOrientRotationToMovement;
    C->bUseControllerRotationYaw = bSavedUseControllerRotationYaw;

    MoveComp->StopMovementImmediately();
    MoveComp->SetMovementMode(MOVE_Walking);
    MoveComp->UpdateComponentVelocity();
}