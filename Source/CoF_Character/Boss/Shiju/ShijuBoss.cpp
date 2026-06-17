#include "ShijuBoss.h"

#include "../Data/ShijuBossDataAsset.h"
#include "ShijuArrowProjectile.h"
#include "ShijuQArea.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AShijuBoss::AShijuBoss()
{
    PrimaryActorTick.bCanEverTick = true;

    CurrentTarget = nullptr;
    ArrowSpawnSocketName = TEXT("BowEmitterSocket");

    CurrentPhase = 0;
    bPhase3Entered = false;

    LastBasicArrowTime = -1000.f;
    LastPiercingShotTime = -1000.f;
    LastQTime = -1000.f;
    LastRTime = -1000.f;

    bAttackInProgress = false;
    CachedQTargetLocation = FVector::ZeroVector;

    PendingProjectileClass = nullptr;
    PendingProjectileDamage = 0.f;
    PendingProjectileSpeed = 0.f;
    bPiercingProjectilePending = false;

    ActiveQWarningArea = nullptr;

    bRSkillActive = false;
    RShotsFiredThisCast = 0;

    CurrentTimeMarkStack = 0;
    SavedHPAtFirstMark = 0.f;
    bBellPassiveCoolingDown = false;
    bNextArrowEmpowered = false;
    NextArrowDamageMultiplier = 1.f;
    NextArrowSpeedMultiplier = 1.f;
}

void AShijuBoss::BeginPlay()
{
    Super::BeginPlay();

    if (UShijuBossDataAsset* Data = GetShijuData())
    {
        CurrentHP = Data->MaxHP;

        UE_LOG(LogTemp, Warning, TEXT("[Shiju Init] HP = %.2f / %.2f"), CurrentHP, Data->MaxHP);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                3.f,
                FColor::Green,
                FString::Printf(TEXT("[Shiju Init] HP = %.0f / %.0f"), CurrentHP, Data->MaxHP)
            );
        }

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = Data->WalkSpeed;
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Shiju Init] Shiju DataAsset is null"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.f,
                FColor::Red,
                TEXT("[Shiju Init] DataAsset is null")
            );
        }
    }

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &AShijuBoss::HandleMontageNotifyBegin);
        }
    }

    UpdatePhaseByHP();
}

void AShijuBoss::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdatePhaseByHP();
}

void AShijuBoss::SetCurrentTarget(AActor* InTarget)
{
    CurrentTarget = InTarget;
}

AActor* AShijuBoss::GetCurrentTarget() const
{
    return CurrentTarget;
}

UShijuBossDataAsset* AShijuBoss::GetShijuData() const
{
    return Cast<UShijuBossDataAsset>(GetBossData());
}

bool AShijuBoss::CanUseBasicArrow() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || !GetWorld())
    {
        return false;
    }

    return (GetWorld()->GetTimeSeconds() - LastBasicArrowTime) >= Data->BasicArrowCooldown;
}

bool AShijuBoss::CanUsePiercingShot() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || !GetWorld())
    {
        return false;
    }

    return (GetWorld()->GetTimeSeconds() - LastPiercingShotTime) >= Data->PiercingShotCooldown;
}

bool AShijuBoss::CanUseQ() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || !GetWorld())
    {
        return false;
    }

    return (GetWorld()->GetTimeSeconds() - LastQTime) >= Data->QCooldown;
}

bool AShijuBoss::CanUseR() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || !GetWorld())
    {
        return false;
    }

    return (GetWorld()->GetTimeSeconds() - LastRTime) >= Data->RCooldown;
}

void AShijuBoss::FaceTarget()
{
    if (!CurrentTarget)
    {
        return;
    }

    const FVector MyLocation = GetActorLocation();
    const FVector TargetLocation = CurrentTarget->GetActorLocation();
    const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MyLocation, TargetLocation);

    SetActorRotation(FRotator(0.f, LookAtRot.Yaw, 0.f));
}

bool AShijuBoss::IsAttackInProgress() const
{
    return bAttackInProgress;
}


void AShijuBoss::EndAttack()
{
    bAttackInProgress = false;
    bPiercingProjectilePending = false;
    bRSkillActive = false;
}

void AShijuBoss::DebugApplyDamageToSelf(float DamageAmount)
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        UE_LOG(LogTemp, Error, TEXT("[Shiju Debug] DataAsset is null"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                3.f,
                FColor::Red,
                TEXT("[Shiju Debug] DataAsset is null")
            );
        }

        return;
    }

    if (DamageAmount <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Debug] DamageAmount must be positive"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                3.f,
                FColor::Yellow,
                TEXT("[Shiju Debug] DamageAmount must be positive")
            );
        }

        return;
    }

    const float OldHP = CurrentHP;
    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, Data->MaxHP);

    const float HPRatio = Data->MaxHP > KINDA_SMALL_NUMBER
        ? CurrentHP / Data->MaxHP
        : 0.f;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Debug] Damage = %.2f, HP %.2f -> %.2f / %.2f, Ratio = %.2f, CurrentPhase = %d"),
        DamageAmount,
        OldHP,
        CurrentHP,
        Data->MaxHP,
        HPRatio,
        CurrentPhase
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Yellow,
            FString::Printf(
                TEXT("[Shiju Debug] Damage %.0f | HP %.0f -> %.0f / %.0f | Ratio %.2f | Phase %d"),
                DamageAmount,
                OldHP,
                CurrentHP,
                Data->MaxHP,
                HPRatio,
                CurrentPhase
            )
        );
    }

    UpdatePhaseByHP();
}

bool AShijuBoss::TryUsePhasePattern()
{
    if (!CurrentTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Pattern] Failed - CurrentTarget is null"));
        return false;
    }

    if (bAttackInProgress)
    {
        return false;
    }

    UpdatePhaseByHP();

    const int32 Phase = FMath::Max(1, CurrentPhase);
    const int32 Roll = FMath::RandRange(1, 100);

    auto LogPattern = [this](const TCHAR* PatternName)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shiju Pattern] Phase %d - %s"), CurrentPhase, PatternName);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    2.f,
                    FColor::Silver,
                    FString::Printf(TEXT("[Shiju Pattern] Phase %d - %s"), CurrentPhase, PatternName)
                );
            }
        };

    auto UseBasic = [this, &LogPattern]() -> bool
        {
            if (PerformBasicArrowAttack())
            {
                LogPattern(TEXT("BasicArrow"));
                return true;
            }

            return false;
        };

    auto UsePiercing = [this, &LogPattern]() -> bool
        {
            if (PerformPiercingShot())
            {
                LogPattern(TEXT("PiercingShot"));
                return true;
            }

            return false;
        };

    auto UseQ = [this, &LogPattern]() -> bool
        {
            if (PerformQTimeRain())
            {
                LogPattern(TEXT("QTimeRain"));
                return true;
            }

            return false;
        };

    auto UseR = [this, &LogPattern]() -> bool
        {
            if (PerformRSkill())
            {
                LogPattern(TEXT("RSkill"));
                return true;
            }

            return false;
        };

    if (Phase <= 1)
    {
        // Phase 1
        // 기본 화살 70%, 관통 화살 30%
        if (Roll <= 70)
        {
            if (UseBasic()) return true;
            if (UsePiercing()) return true;
        }
        else
        {
            if (UsePiercing()) return true;
            if (UseBasic()) return true;
        }
    }
    else if (Phase == 2)
    {
        // Phase 2
        // 기본 화살 35%, 관통 화살 30%, Q 장판 35%
        if (Roll <= 35)
        {
            if (UseBasic()) return true;
            if (UsePiercing()) return true;
            if (UseQ()) return true;
        }
        else if (Roll <= 65)
        {
            if (UsePiercing()) return true;
            if (UseQ()) return true;
            if (UseBasic()) return true;
        }
        else
        {
            if (UseQ()) return true;
            if (UsePiercing()) return true;
            if (UseBasic()) return true;
        }
    }
    else
    {
        // Phase 3
        // 기본 화살 15%, 관통 화살 25%, Q 장판 30%, R 연사 30%
        if (Roll <= 15)
        {
            if (UseBasic()) return true;
            if (UsePiercing()) return true;
            if (UseQ()) return true;
            if (UseR()) return true;
        }
        else if (Roll <= 40)
        {
            if (UsePiercing()) return true;
            if (UseQ()) return true;
            if (UseR()) return true;
            if (UseBasic()) return true;
        }
        else if (Roll <= 70)
        {
            if (UseQ()) return true;
            if (UseR()) return true;
            if (UsePiercing()) return true;
            if (UseBasic()) return true;
        }
        else
        {
            if (UseR()) return true;
            if (UseQ()) return true;
            if (UsePiercing()) return true;
            if (UseBasic()) return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Pattern] Failed - All skills unavailable. Phase = %d"), Phase);
    return false;
}

void AShijuBoss::HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
    UE_LOG(LogTemp, Warning, TEXT("[Shiju Notify] %s"), *NotifyName.ToString());

    if (NotifyName == TEXT("P_Fire"))
    {
        FirePendingPiercingShot();
        return;
    }

    if (NotifyName == TEXT("R_Fire"))
    {
        if (!bRSkillActive)
        {
            return;
        }

        FireRArrowOnce();
        return;
    }
}

bool AShijuBoss::ResolveQGroundTargetLocation(FVector& OutGroundLocation) const
{
    if (!CurrentTarget || !GetWorld())
    {
        return false;
    }

    const FVector TargetLocation = CurrentTarget->GetActorLocation();
    const FVector Start = TargetLocation + FVector(0.f, 0.f, 200.f);
    const FVector End = TargetLocation - FVector(0.f, 0.f, 3000.f);

    FHitResult HitResult;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ShijuQGroundTrace), false);
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(CurrentTarget);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        OutGroundLocation = HitResult.ImpactPoint + FVector(0.f, 0.f, 2.f);
        return true;
    }

    OutGroundLocation = TargetLocation;
    OutGroundLocation.Z = GetActorLocation().Z;
    return false;
}

TSubclassOf<AShijuArrowProjectile> AShijuBoss::GetBasicProjectileClass() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return nullptr;
    }

    if (Data->BasicArrowProjectileClass)
    {
        return Data->BasicArrowProjectileClass;
    }

    return Data->ArrowProjectileClass;
}

TSubclassOf<AShijuArrowProjectile> AShijuBoss::GetPiercingProjectileClass() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return nullptr;
    }

    if (Data->PiercingProjectileClass)
    {
        return Data->PiercingProjectileClass;
    }

    if (Data->BasicArrowProjectileClass)
    {
        return Data->BasicArrowProjectileClass;
    }

    return Data->ArrowProjectileClass;
}

TSubclassOf<AShijuArrowProjectile> AShijuBoss::GetQProjectileClass() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return nullptr;
    }

    if (Data->QProjectileClass)
    {
        return Data->QProjectileClass;
    }

    if (Data->BasicArrowProjectileClass)
    {
        return Data->BasicArrowProjectileClass;
    }

    return Data->ArrowProjectileClass;
}

TSubclassOf<AShijuArrowProjectile> AShijuBoss::GetRProjectileClass() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return nullptr;
    }

    if (Data->RProjectileClass)
    {
        return Data->RProjectileClass;
    }

    if (Data->PiercingProjectileClass)
    {
        return Data->PiercingProjectileClass;
    }

    if (Data->BasicArrowProjectileClass)
    {
        return Data->BasicArrowProjectileClass;
    }

    return Data->ArrowProjectileClass;
}

AShijuArrowProjectile* AShijuBoss::SpawnProjectileTowardsCurrentTarget(
    TSubclassOf<AShijuArrowProjectile> InProjectileClass,
    float InDamage,
    float InSpeed
)
{
    if (!InProjectileClass)
    {
        return nullptr;
    }

    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 120.f + FVector(0.f, 0.f, 60.f);
    FRotator SpawnRotation = GetActorRotation();

    if (USkeletalMeshComponent* SkelMesh = GetMesh())
    {
        if (SkelMesh->DoesSocketExist(ArrowSpawnSocketName))
        {
            SpawnLocation = SkelMesh->GetSocketLocation(ArrowSpawnSocketName);
            SpawnRotation = SkelMesh->GetSocketRotation(ArrowSpawnSocketName);
        }
    }

    if (CurrentTarget)
    {
        const FVector AimLocation = CurrentTarget->GetActorLocation() + FVector(0.f, 0.f, 60.f);
        SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, AimLocation);
    }

    SpawnLocation += SpawnRotation.Vector() * 120.f;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AShijuArrowProjectile* Arrow = GetWorld()->SpawnActor<AShijuArrowProjectile>(
        InProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (Arrow)
    {
        float FinalDamage = InDamage;
        float FinalSpeed = InSpeed;

        ApplyAndConsumeNextArrowEmpower(FinalDamage, FinalSpeed);

        Arrow->InitProjectile(FinalDamage, this, FinalSpeed);
    }
    return Arrow;
}

void AShijuBoss::QueueProjectileSpawn(TSubclassOf<AShijuArrowProjectile> InProjectileClass, float InDamage, float InSpeed, float Delay)
{
    if (!GetWorld() || !InProjectileClass)
    {
        return;
    }

    PendingProjectileClass = InProjectileClass;
    PendingProjectileDamage = InDamage;
    PendingProjectileSpeed = InSpeed;

    FTimerHandle SpawnHandle;
    GetWorldTimerManager().SetTimer(
        SpawnHandle,
        this,
        &AShijuBoss::SpawnQueuedProjectile,
        Delay,
        false
    );
}

void AShijuBoss::SpawnQueuedProjectile()
{
    if (!PendingProjectileClass)
    {
        return;
    }

    SpawnProjectileTowardsCurrentTarget(PendingProjectileClass, PendingProjectileDamage, PendingProjectileSpeed);

    PendingProjectileClass = nullptr;
    PendingProjectileDamage = 0.f;
    PendingProjectileSpeed = 0.f;
}

bool AShijuBoss::PerformBasicArrowAttack()
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || !CurrentTarget || bAttackInProgress || !CanUseBasicArrow())
    {
        return false;
    }

    bAttackInProgress = true;
    FaceTarget();


    float AttackLockTime = 0.5f;

    if (Data->BasicArrowMontage)
    {
        const float PlayedLength = PlayAnimMontage(Data->BasicArrowMontage);
        if (PlayedLength > 0.f)
        {
            AttackLockTime = PlayedLength;
        }
    }

    QueueProjectileSpawn(GetBasicProjectileClass(), Data->BasicArrowDamage, Data->ArrowSpeed, 0.15f);

    FTimerHandle AttackEndHandle;
    GetWorldTimerManager().SetTimer(
        AttackEndHandle,
        this,
        &AShijuBoss::EndAttack,
        AttackLockTime,
        false
    );

    LastBasicArrowTime = GetWorld()->GetTimeSeconds();
    return true;
}

bool AShijuBoss::PerformPiercingShot()
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || !CurrentTarget || bAttackInProgress || !CanUsePiercingShot())
    {
        return false;
    }

    bAttackInProgress = true;
    FaceTarget();

    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->StopMovement();
    }

    float AttackLockTime = Data->PiercingChargeTime;

    if (Data->PiercingShotMontage)
    {
        const float PlayedLength = PlayAnimMontage(Data->PiercingShotMontage);
        if (PlayedLength > 0.f)
        {
            AttackLockTime = PlayedLength;
        }
    }

    // 실제 발사는 타이머가 아니라 Notify에서 한다.
    PendingProjectileClass = GetPiercingProjectileClass();
    PendingProjectileDamage = Data->PiercingArrowDamage;
    PendingProjectileSpeed = Data->PiercingProjectileSpeed;
    bPiercingProjectilePending = true;

    FTimerHandle AttackEndHandle;
    GetWorldTimerManager().SetTimer(
        AttackEndHandle,
        this,
        &AShijuBoss::EndAttack,
        AttackLockTime,
        false
    );

    LastPiercingShotTime = GetWorld()->GetTimeSeconds();

    UE_LOG(LogTemp, Log, TEXT("[Shiju] PiercingShot Start"));
    return true;
}

bool AShijuBoss::PerformQTimeRain()
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || !CurrentTarget || bAttackInProgress || !CanUseQ())
    {
        return false;
    }

    FVector GroundTargetLocation = FVector::ZeroVector;
    ResolveQGroundTargetLocation(GroundTargetLocation);
    CachedQTargetLocation = GroundTargetLocation;

    if (!Data->QAreaActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju] QAreaActorClass is null"));
        return false;
    }

    if (ActiveQWarningArea)
    {
        ActiveQWarningArea->Destroy();
        ActiveQWarningArea = nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    ActiveQWarningArea = GetWorld()->SpawnActor<AShijuQArea>(
        Data->QAreaActorClass,
        CachedQTargetLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (ActiveQWarningArea)
    {
        ActiveQWarningArea->InitWarning(
            Data->QRadius,
            Data->QDamage,
            Data->QBurnDuration,
            Data->QBurnTickInterval,
            this
        );
    }

    bAttackInProgress = true;
    FaceTarget();

    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->StopMovement();
    }

    float AttackLockTime = Data->QCastDelay + 1.0f;

    if (Data->QMontage)
    {
        const float PlayedLength = PlayAnimMontage(Data->QMontage);
        if (PlayedLength > 0.f)
        {
            AttackLockTime = FMath::Max(AttackLockTime, PlayedLength);
        }
    }

    FTimerHandle ProjectileHandle;
    GetWorldTimerManager().SetTimer(
        ProjectileHandle,
        this,
        &AShijuBoss::SpawnQProjectileArc,
        Data->QCastDelay,
        false
    );

    FTimerHandle EndHandle;
    GetWorldTimerManager().SetTimer(
        EndHandle,
        this,
        &AShijuBoss::EndAttack,
        AttackLockTime,
        false
    );

    LastQTime = GetWorld()->GetTimeSeconds();
    return true;
}

void AShijuBoss::SpawnQProjectileArc()
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return;
    }

    TSubclassOf<AShijuArrowProjectile> QProjectileClass = GetQProjectileClass();
    if (!QProjectileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju] QProjectileClass is null"));
        return;
    }

    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 120.f + FVector(0.f, 0.f, 60.f);
    FRotator SpawnRotation = GetActorRotation();

    if (USkeletalMeshComponent* SkelMesh = GetMesh())
    {
        if (SkelMesh->DoesSocketExist(ArrowSpawnSocketName))
        {
            SpawnLocation = SkelMesh->GetSocketLocation(ArrowSpawnSocketName);
            SpawnRotation = SkelMesh->GetSocketRotation(ArrowSpawnSocketName);
        }
    }

    SpawnLocation += SpawnRotation.Vector() * 120.f;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AShijuArrowProjectile* Arrow = GetWorld()->SpawnActor<AShijuArrowProjectile>(
        QProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (!Arrow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju] Failed to spawn Q projectile"));
        return;
    }

    Arrow->InitProjectile(0.f, this, Data->ArrowSpeed);
    Arrow->InitQProjectile(ActiveQWarningArea);

    UProjectileMovementComponent* ProjectileMove = Arrow->GetProjectileMovementComponent();
    if (!ProjectileMove)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju] Q projectile has no ProjectileMovementComponent"));
        return;
    }

    const FVector Start = SpawnLocation;
    const FVector End = CachedQTargetLocation;
    const float GravityZ = FMath::Abs(GetWorld()->GetGravityZ() * ProjectileMove->ProjectileGravityScale);

    if (GravityZ <= KINDA_SMALL_NUMBER)
    {
        const FVector Dir = (End - Start).GetSafeNormal();
        ProjectileMove->Velocity = Dir * Data->ArrowSpeed;
        return;
    }

    const float ApexZ = FMath::Max(Start.Z, End.Z) + Data->QArcHeight;
    const float UpDistance = FMath::Max(1.f, ApexZ - Start.Z);
    const float DownDistance = FMath::Max(1.f, ApexZ - End.Z);

    const float TimeUp = FMath::Sqrt((2.f * UpDistance) / GravityZ);
    const float TimeDown = FMath::Sqrt((2.f * DownDistance) / GravityZ);
    const float TotalTime = FMath::Max(0.05f, TimeUp + TimeDown);

    const FVector FlatDelta(End.X - Start.X, End.Y - Start.Y, 0.f);

    FVector LaunchVelocity = FVector::ZeroVector;
    LaunchVelocity.X = FlatDelta.X / TotalTime;
    LaunchVelocity.Y = FlatDelta.Y / TotalTime;
    LaunchVelocity.Z = GravityZ * TimeUp;

    ProjectileMove->Velocity = LaunchVelocity;
}

bool AShijuBoss::PerformRSkill()
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || !CurrentTarget || bAttackInProgress || !CanUseR())
    {
        return false;
    }

    if (!Data->RMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju] RMontage is null"));
        return false;
    }

    bAttackInProgress = true;
    bRSkillActive = true;
    RShotsFiredThisCast = 0;

    FaceTarget();

    if (AAIController* AIController = Cast<AAIController>(GetController()))
    {
        AIController->StopMovement();
    }

    float AttackLockTime = Data->RStartDelay + (Data->RFireInterval * FMath::Max(0, Data->RArrowCount - 1)) + 0.3f;

    const float PlayedLength = PlayAnimMontage(Data->RMontage);
    if (PlayedLength > 0.f)
    {
        AttackLockTime = FMath::Max(AttackLockTime, PlayedLength);
    }

    FTimerHandle AttackEndHandle;
    GetWorldTimerManager().SetTimer(
        AttackEndHandle,
        this,
        &AShijuBoss::EndAttack,
        AttackLockTime,
        false
    );

    LastRTime = GetWorld()->GetTimeSeconds();
    return true;
}

void AShijuBoss::FireRArrowOnce()
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return;
    }

    if (RShotsFiredThisCast >= Data->RArrowCount)
    {
        bRSkillActive = false;
        return;
    }

    SpawnProjectileTowardsCurrentTarget(
        GetRProjectileClass(),
        Data->RArrowDamage,
        Data->RProjectileSpeed
    );

    ++RShotsFiredThisCast;

    UE_LOG(LogTemp, Warning, TEXT("[Shiju R] Fired Shot %d"), RShotsFiredThisCast);

    if (RShotsFiredThisCast >= Data->RArrowCount)
    {
        bRSkillActive = false;
    }
}

void AShijuBoss::RegisterTimeMarkHit()
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return;
    }

    if (bBellPassiveCoolingDown)
    {
        return;
    }

    const int32 CurrentMaxMarkStack = GetCurrentMaxTimeMarkStack();

    if (CurrentTimeMarkStack <= 0)
    {
        SavedHPAtFirstMark = CurrentHP;
    }

    CurrentTimeMarkStack = FMath::Clamp(CurrentTimeMarkStack + 1, 0, CurrentMaxMarkStack);

    GetWorldTimerManager().ClearTimer(TimeMarkResetTimerHandle);
    GetWorldTimerManager().SetTimer(
        TimeMarkResetTimerHandle,
        this,
        &AShijuBoss::ResetTimeMarkStack,
        Data->TimeMarkDuration,
        false
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Passive] TimeMark Stack = %d / %d"),
        CurrentTimeMarkStack,
        CurrentMaxMarkStack
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Cyan,
            FString::Printf(TEXT("[Shiju Passive] Mark %d / %d"), CurrentTimeMarkStack, CurrentMaxMarkStack)
        );
    }

    if (CurrentTimeMarkStack >= CurrentMaxMarkStack)
    {
        TriggerBellPassive();
    }
}

void AShijuBoss::ResetTimeMarkStack()
{
    CurrentTimeMarkStack = 0;
    SavedHPAtFirstMark = 0.f;
    GetWorldTimerManager().ClearTimer(TimeMarkResetTimerHandle);

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Passive] TimeMark Reset"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.f,
            FColor::Cyan,
            TEXT("[Shiju Passive] TimeMark Reset")
        );
    }
}

void AShijuBoss::TriggerBellPassive()
{
    UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return;
    }

    if (Data->BellTriggerSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Data->BellTriggerSound, GetActorLocation());
    }

    CurrentHP = FMath::Clamp(SavedHPAtFirstMark, 0.f, Data->MaxHP);

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Passive] Bell Triggered. HP rewound to %.2f"), CurrentHP);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            4.f,
            FColor::Purple,
            FString::Printf(TEXT("[Shiju Passive] Bell Triggered | HP Rewind %.0f"), CurrentHP)
        );
    }

    ResetTimeMarkStack();

    bBellPassiveCoolingDown = true;
    GetWorldTimerManager().SetTimer(
        BellPassiveCooldownTimerHandle,
        this,
        &AShijuBoss::EndBellPassiveCooldown,
        Data->BellPassiveCooldown,
        false
    );
}

void AShijuBoss::EndBellPassiveCooldown()
{
    bBellPassiveCoolingDown = false;
    UE_LOG(LogTemp, Warning, TEXT("[Shiju Passive] Bell Cooldown End"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Cyan,
            TEXT("[Shiju Passive] Bell Cooldown End")
        );
    }
}

void AShijuBoss::UpdatePhaseByHP()
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || Data->MaxHP <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const float HPRatio = FMath::Clamp(CurrentHP / Data->MaxHP, 0.f, 1.f);

    int32 NewPhase = 1;

    if (HPRatio <= Data->Phase3StartHPRatio)
    {
        NewPhase = 3;
    }
    else if (HPRatio <= Data->Phase2StartHPRatio)
    {
        NewPhase = 2;
    }

    if (CurrentPhase == NewPhase)
    {
        return;
    }

    const int32 OldPhase = CurrentPhase;
    CurrentPhase = NewPhase;

    HandlePhaseChanged(OldPhase, NewPhase);
}

void AShijuBoss::HandlePhaseChanged(int32 OldPhase, int32 NewPhase)
{
    UE_LOG(LogTemp, Warning, TEXT("[Shiju Phase] %d -> %d"), OldPhase, NewPhase);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Orange,
            FString::Printf(TEXT("[Shiju Phase] %d -> %d"), OldPhase, NewPhase)
        );
    }

    if (NewPhase >= 3 && !bPhase3Entered)
    {
        bPhase3Entered = true;
        EnterPhase3();
    }
}

void AShijuBoss::EnterPhase3()
{
    UE_LOG(LogTemp, Warning, TEXT("[Shiju Phase3] Enter Phase 3"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Red,
            TEXT("[Shiju Phase3] Enter Phase 3")
        );
    }

    ResetTimeMarkStack();
    RemovePhase3Pillars();
}

void AShijuBoss::RemovePhase3Pillars()
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data || Data->Phase3RemovePillarTag.IsNone() || !GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Phase3] Cannot remove pillars. Data or Tag is invalid"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                4.f,
                FColor::Red,
                TEXT("[Shiju Phase3] Pillar tag invalid")
            );
        }

        return;
    }

    TArray<AActor*> Pillars;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), Data->Phase3RemovePillarTag, Pillars);

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Phase3] Pillars Found: %d"), Pillars.Num());

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            4.f,
            FColor::Red,
            FString::Printf(TEXT("[Shiju Phase3] Pillars Found: %d"), Pillars.Num())
        );
    }

    for (AActor* Pillar : Pillars)
    {
        if (!Pillar || Pillar == this)
        {
            continue;
        }

        UE_LOG(LogTemp, Warning, TEXT("[Shiju Phase3] Remove Pillar: %s"), *Pillar->GetName());
        Pillar->Destroy();
    }
}

int32 AShijuBoss::GetCurrentMaxTimeMarkStack() const
{
    const UShijuBossDataAsset* Data = GetShijuData();
    if (!Data)
    {
        return 4;
    }

    if (CurrentPhase >= 3)
    {
        return Data->Phase3MaxTimeMarkStack;
    }

    return Data->MaxTimeMarkStack;
}

void AShijuBoss::FirePendingPiercingShot()
{
    if (!bPiercingProjectilePending)
    {
        return;
    }

    bPiercingProjectilePending = false;

    UE_LOG(LogTemp, Log, TEXT("[Shiju] FirePendingPiercingShot"));

    SpawnQueuedProjectile();
}
int32 AShijuBoss::GetCurrentPhase() const
{
    return CurrentPhase;
}

void AShijuBoss::EmpowerNextArrow(float InDamageMultiplier, float InSpeedMultiplier)
{
    bNextArrowEmpowered = true;
    NextArrowDamageMultiplier = FMath::Max(1.f, InDamageMultiplier);
    NextArrowSpeedMultiplier = FMath::Max(1.f, InSpeedMultiplier);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Bell] Next Arrow Empowered | Damage x%.2f | Speed x%.2f"),
        NextArrowDamageMultiplier,
        NextArrowSpeedMultiplier
    );
}

void AShijuBoss::ApplyAndConsumeNextArrowEmpower(float& InOutDamage, float& InOutSpeed)
{
    if (!bNextArrowEmpowered)
    {
        return;
    }

    // Q 낙하 화살처럼 데미지가 0인 투사체에는 강화 소모하지 않음.
    if (InOutDamage <= 0.f)
    {
        return;
    }

    InOutDamage *= NextArrowDamageMultiplier;
    InOutSpeed *= NextArrowSpeedMultiplier;

    bNextArrowEmpowered = false;
    NextArrowDamageMultiplier = 1.f;
    NextArrowSpeedMultiplier = 1.f;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Bell] Next Arrow Empower Consumed | Damage %.2f | Speed %.2f"),
        InOutDamage,
        InOutSpeed
    );
}