#include "Skills/Kallari/Kallari_Skill1B_Backflip.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "TP_Character.h"

void UKallari_Skill1B_Backflip::DashStart()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;

    UCharacterMovementComponent* Move = C->GetCharacterMovement();
    if (!Move || bDashMoving) return;

    bDashMoving = true;

    // 기존 저장값
    SavedGroundFriction = Move->GroundFriction;
    SavedBrakingFrictionFactor = Move->BrakingFrictionFactor;
    SavedBrakingDecelerationWalking = Move->BrakingDecelerationWalking;
    SavedBrakingDecelerationFlying = Move->BrakingDecelerationFlying;
    bSavedOrientRotationToMovement = Move->bOrientRotationToMovement;
    bSavedUseControllerRotationYaw = C->bUseControllerRotationYaw;

    // 공중제비 중에는 회전만 잠시 고정
    Move->bOrientRotationToMovement = false;
    C->bUseControllerRotationYaw = false;

    const float Duration =
        (C->Skill1B_BackflipDuration > KINDA_SMALL_NUMBER)
        ? C->Skill1B_BackflipDuration
        : 0.01f;

    const float BackwardSpeed = C->Skill1B_BackwardDistance / Duration;
    const float UpwardSpeed = C->Skill1B_UpwardDistance / Duration;

    // Falling 상태에서 뒤 + 위 방향 속도를 준다
    Move->SetMovementMode(MOVE_Falling);
    Move->Velocity =
        (-C->GetActorForwardVector() * BackwardSpeed) +
        (FVector::UpVector * UpwardSpeed);

    Move->UpdateComponentVelocity();
}

void UKallari_Skill1B_Backflip::DashEnd()
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

    MoveComp->UpdateComponentVelocity();
}