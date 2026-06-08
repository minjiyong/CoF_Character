#include "BossBase.h"

#include "AI/BossAIController.h"
#include "Data/BossDataAsset.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

ABossBase::ABossBase()
{
    PrimaryActorTick.bCanEverTick = false;

    AIControllerClass = ABossAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    CurrentHP = 0.f;
    bDead = false;
    bAttacking = false;
    LastAttackTime = -1000.f;
}

void ABossBase::BeginPlay()
{
    Super::BeginPlay();
    ApplyBossData();
}

void ABossBase::ApplyBossData()
{
    if (!BossData) return;

    CurrentHP = BossData->MaxHP;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = BossData->WalkSpeed;
    }
}

float ABossBase::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (bDead || AppliedDamage <= 0.f)
    {
        return 0.f;
    }

    CurrentHP = FMath::Max(0.f, CurrentHP - AppliedDamage);

    if (CurrentHP <= 0.f)
    {
        Die();
        return AppliedDamage;
    }

    if (BossData && BossData->HitReactMontage)
    {
        PlayAnimMontage(BossData->HitReactMontage);
    }

    return AppliedDamage;
}

bool ABossBase::CanAttack() const
{
    if (bDead) return false;
    if (bAttacking) return false;
    if (!BossData) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    return (World->GetTimeSeconds() - LastAttackTime) >= BossData->AttackCooldown;
}

void ABossBase::StartAttack()
{
    if (!CanAttack()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    bAttacking = true;
    LastAttackTime = World->GetTimeSeconds();

    World->GetTimerManager().ClearTimer(AttackFinishTimer);

    float FinishDelay = 0.3f;
    bool bMontagePlayed = false;

    if (BossData && BossData->AttackMontage)
    {
        if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &ABossBase::OnAttackMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, BossData->AttackMontage);

            const float PlayedLength = PlayAnimMontage(BossData->AttackMontage);
            if (PlayedLength > 0.f)
            {
                bMontagePlayed = true;
                FinishDelay = PlayedLength;
            }
        }
    }

    if (!bMontagePlayed && BossData)
    {
        FinishDelay = FMath::Clamp(BossData->AttackCooldown * 0.5f, 0.15f, 0.8f);
    }

    World->GetTimerManager().SetTimer(
        AttackFinishTimer,
        this,
        &ABossBase::FinishAttack,
        FinishDelay,
        false
    );
}

void ABossBase::MeleeAttack()
{
    StartAttack();
}

void ABossBase::FinishAttack()
{
    bAttacking = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AttackFinishTimer);
    }
}

void ABossBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    FinishAttack();
}

void ABossBase::Die()
{
    if (bDead) return;

    bDead = true;
    bAttacking = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AttackFinishTimer);
    }

    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->StopMovement();

        if (UBrainComponent* Brain = AI->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("BossDead"));
        }
    }

    if (BossData && BossData->DeadMontage)
    {
        PlayAnimMontage(BossData->DeadMontage);
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    SetActorEnableCollision(false);

    OnBossDead();
}

float ABossBase::GetHPPercent() const
{
    if (!BossData || BossData->MaxHP <= 0.f)
    {
        return 0.f;
    }

    return CurrentHP / BossData->MaxHP;
}