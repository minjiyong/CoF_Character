#include "ShijuBellShotManager.h"

#include "../Shiju/ShijuArrowProjectile.h"
#include "../Shiju/ShijuBoss.h"
#include "ShijuBellTarget.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AShijuBellShotManager::AShijuBellShotManager()
{
    PrimaryActorTick.bCanEverTick = true;

    ShijuBoss = nullptr;
    BellShotProjectileClass = nullptr;

    ArrowSpawnSocketName = TEXT("BowEmitterSocket");

    bAutoFire = true;
    InitialDelay = 3.f;

    Phase2FireInterval = 8.f;
    Phase3FireInterval = 5.f;

    ProjectileSpeed = 4200.f;
    ProjectileDamage = 1.f;

    bRotateBossTowardBell = true;
    bDebugLog = true;

    LastBellShotTime = -1000.f;
}

void AShijuBellShotManager::BeginPlay()
{
    Super::BeginPlay();

    FindShijuBossIfNeeded();

    if (bDebugLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju BellShot] BeginPlay"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                3.f,
                FColor::Yellow,
                TEXT("[Shiju BellShot] BeginPlay")
            );
        }
    }
}

void AShijuBellShotManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateBellShot();
}

void AShijuBellShotManager::FindShijuBossIfNeeded()
{
    if (ShijuBoss || !GetWorld())
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShijuBoss::StaticClass(), FoundActors);

    if (FoundActors.Num() <= 0)
    {
        return;
    }

    ShijuBoss = Cast<AShijuBoss>(FoundActors[0]);

    if (bDebugLog && ShijuBoss)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju BellShot] Found ShijuBoss: %s"), *ShijuBoss->GetName());
    }
}

void AShijuBellShotManager::UpdateBellShot()
{
    if (!bAutoFire || !GetWorld())
    {
        return;
    }

    FindShijuBossIfNeeded();

    if (!ShijuBoss)
    {
        return;
    }

    const int32 CurrentPhase = ShijuBoss->GetCurrentPhase();

    if (CurrentPhase < 2)
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    if (CurrentTime < InitialDelay)
    {
        return;
    }

    const float RequiredInterval = CurrentPhase >= 3
        ? Phase3FireInterval
        : Phase2FireInterval;

    if ((CurrentTime - LastBellShotTime) < RequiredInterval)
    {
        return;
    }

    FireBellShot();
    LastBellShotTime = CurrentTime;
}

void AShijuBellShotManager::FireBellShot()
{
    FindShijuBossIfNeeded();

    if (!GetWorld() || !ShijuBoss || !BellShotProjectileClass)
    {
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shiju BellShot] Failed. Boss or ProjectileClass is null"));
        }

        return;
    }

    AShijuBellTarget* BellTarget = ChooseRandomBellTarget();
    if (!BellTarget)
    {
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shiju BellShot] No valid bell target"));
        }

        return;
    }

    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;

    if (!ResolveBellShotSpawnTransform(BellTarget, SpawnLocation, SpawnRotation))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju BellShot] Failed to resolve spawn transform"));
        return;
    }

    if (bRotateBossTowardBell)
    {
        ShijuBoss->SetActorRotation(FRotator(0.f, SpawnRotation.Yaw, 0.f));
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = ShijuBoss;
    SpawnParams.Instigator = ShijuBoss;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AShijuArrowProjectile* Arrow = GetWorld()->SpawnActor<AShijuArrowProjectile>(
        BellShotProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (!Arrow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju BellShot] Failed to spawn arrow"));
        return;
    }

    Arrow->InitProjectile(ProjectileDamage, ShijuBoss, ProjectileSpeed);

    if (bDebugLog)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Shiju BellShot] Fire to Bell: %s | Phase = %d | Spawn = %s"),
            *BellTarget->GetName(),
            ShijuBoss ? ShijuBoss->GetCurrentPhase() : -1,
            *SpawnLocation.ToString()
        );

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                3.f,
                FColor::Yellow,
                FString::Printf(
                    TEXT("[Shiju BellShot] Phase %d -> %s"),
                    ShijuBoss ? ShijuBoss->GetCurrentPhase() : -1,
                    *BellTarget->GetName()
                )
            );
        }
    }
}

AShijuBellTarget* AShijuBellShotManager::ChooseRandomBellTarget() const
{
    if (!GetWorld())
    {
        return nullptr;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShijuBellTarget::StaticClass(), FoundActors);

    TArray<AShijuBellTarget*> ValidBells;

    for (AActor* Actor : FoundActors)
    {
        AShijuBellTarget* Bell = Cast<AShijuBellTarget>(Actor);
        if (!Bell)
        {
            continue;
        }

        if (!Bell->CanBeShot())
        {
            continue;
        }

        ValidBells.Add(Bell);
    }

    if (ValidBells.Num() <= 0)
    {
        return nullptr;
    }

    return ValidBells[FMath::RandRange(0, ValidBells.Num() - 1)];
}

bool AShijuBellShotManager::ResolveBellShotSpawnTransform(
    AShijuBellTarget* BellTarget,
    FVector& OutSpawnLocation,
    FRotator& OutSpawnRotation
) const
{
    if (!ShijuBoss || !BellTarget)
    {
        return false;
    }

    OutSpawnLocation =
        ShijuBoss->GetActorLocation() +
        ShijuBoss->GetActorForwardVector() * 120.f +
        FVector(0.f, 0.f, 80.f);

    if (USkeletalMeshComponent* BossMesh = ShijuBoss->GetMesh())
    {
        if (BossMesh->DoesSocketExist(ArrowSpawnSocketName))
        {
            OutSpawnLocation = BossMesh->GetSocketLocation(ArrowSpawnSocketName);
        }
    }

    const FVector AimLocation = BellTarget->GetBellAimLocation();
    OutSpawnRotation = UKismetMathLibrary::FindLookAtRotation(OutSpawnLocation, AimLocation);

    OutSpawnLocation += OutSpawnRotation.Vector() * 100.f;

    return true;
}