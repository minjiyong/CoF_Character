#include "ShijuArenaManager.h"

#include "../Shiju/ShijuBoss.h"
#include "ShijuBellWaveActor.h"
#include "ShijuTimeRiftZone.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AShijuArenaManager::AShijuArenaManager()
{
    PrimaryActorTick.bCanEverTick = true;

    ShijuBoss = nullptr;
    TimeRiftZoneClass = nullptr;
    BellWaveActorClass = nullptr;

    SpawnRadius = 1600.f;

    bSpawnTimeRiftAroundPlayer = true;
    TimeRiftMinDistanceFromPlayer = 300.f;
    TimeRiftMaxDistanceFromPlayer = 900.f;
    bFallbackToArenaCenterIfNoPlayer = true;

    GroundTraceHeight = 1000.f;
    GroundTraceDepth = 3000.f;

    MaxActiveRifts = 2;

    Phase2SpawnInterval = 8.f;
    Phase3SpawnInterval = 4.f;

    TimeRiftRadius = 300.f;
    TimeRiftDuration = 6.f;
    TimeRiftSlowMultiplier = 0.7f;

    bEnableBellWave = true;
    Phase3BellWaveInterval = 12.f;
    BellWaveMaxRadius = 1800.f;
    BellWaveExpansionDuration = 2.0f;
    BellWaveDamage = 0.f;
    bBellWaveAppliesTimeMark = true;

    bDebugLog = true;
    bDrawSpawnArea = true;

    LastTimeRiftSpawnTime = -1000.f;
    LastBellWaveSpawnTime = -1000.f;
}

void AShijuArenaManager::BeginPlay()
{
    Super::BeginPlay();

    FindShijuBossIfNeeded();

    if (bDebugLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Arena] BeginPlay"));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                3.f,
                FColor::Cyan,
                TEXT("[Shiju Arena] BeginPlay")
            );
        }
    }
}

void AShijuArenaManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    FindShijuBossIfNeeded();
    CleanupInvalidRifts();

    if (bDrawSpawnArea && GetWorld())
    {
        if (bSpawnTimeRiftAroundPlayer)
        {
            ACharacter* TargetPlayer = ChoosePlayerForTimeRift();
            if (TargetPlayer)
            {
                DrawDebugCircle(
                    GetWorld(),
                    TargetPlayer->GetActorLocation(),
                    TimeRiftMinDistanceFromPlayer,
                    48,
                    FColor::Cyan,
                    false,
                    0.f,
                    0,
                    2.f,
                    FVector(1.f, 0.f, 0.f),
                    FVector(0.f, 1.f, 0.f),
                    false
                );

                DrawDebugCircle(
                    GetWorld(),
                    TargetPlayer->GetActorLocation(),
                    TimeRiftMaxDistanceFromPlayer,
                    64,
                    FColor::Blue,
                    false,
                    0.f,
                    0,
                    3.f,
                    FVector(1.f, 0.f, 0.f),
                    FVector(0.f, 1.f, 0.f),
                    false
                );
            }
        }
        else
        {
            DrawDebugCircle(
                GetWorld(),
                GetActorLocation(),
                SpawnRadius,
                64,
                FColor::Blue,
                false,
                0.f,
                0,
                3.f,
                FVector(1.f, 0.f, 0.f),
                FVector(0.f, 1.f, 0.f),
                false
            );
        }
    }

    UpdateTimeRiftSpawn();
    UpdateBellWaveSpawn();
}

void AShijuArenaManager::FindShijuBossIfNeeded()
{
    if (ShijuBoss)
    {
        return;
    }

    if (!GetWorld())
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
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Arena] Found ShijuBoss: %s"), *ShijuBoss->GetName());
    }
}

void AShijuArenaManager::UpdateTimeRiftSpawn()
{
    if (!GetWorld() || !ShijuBoss || !TimeRiftZoneClass)
    {
        return;
    }

    const int32 CurrentPhase = ShijuBoss->GetCurrentPhase();

    if (CurrentPhase < 2)
    {
        return;
    }

    CleanupInvalidRifts();

    if (ActiveRifts.Num() >= MaxActiveRifts)
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float SpawnInterval = CurrentPhase >= 3 ? Phase3SpawnInterval : Phase2SpawnInterval;

    if ((CurrentTime - LastTimeRiftSpawnTime) < SpawnInterval)
    {
        return;
    }

    SpawnTimeRift();
    LastTimeRiftSpawnTime = CurrentTime;
}

void AShijuArenaManager::SpawnTimeRift()
{
    if (!GetWorld() || !TimeRiftZoneClass)
    {
        return;
    }

    FVector SpawnLocation = FVector::ZeroVector;
    if (!ResolveRandomGroundLocation(SpawnLocation))
    {
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shiju Arena] Failed to resolve TimeRift ground location"));
        }

        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AShijuTimeRiftZone* TimeRift = GetWorld()->SpawnActor<AShijuTimeRiftZone>(
        TimeRiftZoneClass,
        SpawnLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (!TimeRift)
    {
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shiju Arena] Failed to spawn TimeRift"));
        }

        return;
    }

    TimeRift->InitTimeRift(
        TimeRiftRadius,
        TimeRiftDuration,
        TimeRiftSlowMultiplier
    );

    ActiveRifts.Add(TimeRift);

    if (bDebugLog)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Shiju Arena] Spawn TimeRift | Phase = %d | Active = %d | Location = %s | AroundPlayer = %s"),
            ShijuBoss ? ShijuBoss->GetCurrentPhase() : -1,
            ActiveRifts.Num(),
            *SpawnLocation.ToString(),
            bSpawnTimeRiftAroundPlayer ? TEXT("true") : TEXT("false")
        );

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                3.f,
                FColor::Cyan,
                FString::Printf(
                    TEXT("[Shiju Arena] Spawn TimeRift | Phase %d | Active %d"),
                    ShijuBoss ? ShijuBoss->GetCurrentPhase() : -1,
                    ActiveRifts.Num()
                )
            );
        }
    }
}

ACharacter* AShijuArenaManager::ChoosePlayerForTimeRift() const
{
    if (!GetWorld())
    {
        return nullptr;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

    TArray<ACharacter*> PlayerCharacters;

    for (AActor* Actor : FoundActors)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (!Character)
        {
            continue;
        }

        if (!Character->IsPlayerControlled())
        {
            continue;
        }

        PlayerCharacters.Add(Character);
    }

    if (PlayerCharacters.Num() <= 0)
    {
        return nullptr;
    }

    return PlayerCharacters[FMath::RandRange(0, PlayerCharacters.Num() - 1)];
}

bool AShijuArenaManager::ResolveRandomGroundLocation(FVector& OutLocation) const
{
    if (bSpawnTimeRiftAroundPlayer)
    {
        ACharacter* TargetPlayer = ChoosePlayerForTimeRift();
        if (TargetPlayer && ResolveRandomGroundLocationNearPlayer(TargetPlayer, OutLocation))
        {
            return true;
        }

        if (!bFallbackToArenaCenterIfNoPlayer)
        {
            return false;
        }

        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shiju Arena] Fallback to arena center TimeRift spawn"));
        }
    }

    return ResolveRandomGroundLocationInArena(OutLocation);
}

bool AShijuArenaManager::ResolveRandomGroundLocationNearPlayer(ACharacter* TargetPlayer, FVector& OutLocation) const
{
    if (!GetWorld() || !TargetPlayer)
    {
        return false;
    }

    const FVector PlayerLocation = TargetPlayer->GetActorLocation();

    const float MinDistance = FMath::Max(0.f, TimeRiftMinDistanceFromPlayer);
    const float MaxDistance = FMath::Max(MinDistance + 1.f, TimeRiftMaxDistanceFromPlayer);

    for (int32 TryIndex = 0; TryIndex < 12; ++TryIndex)
    {
        const float Angle = FMath::FRandRange(0.f, 2.f * PI);

        // 원판이 아니라 도넛 영역에서 비교적 균등하게 뽑기 위한 거리 계산.
        const float MinDistanceSquared = FMath::Square(MinDistance);
        const float MaxDistanceSquared = FMath::Square(MaxDistance);
        const float Distance = FMath::Sqrt(FMath::FRandRange(MinDistanceSquared, MaxDistanceSquared));

        const FVector Offset(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.f
        );

        const FVector Candidate = PlayerLocation + Offset;
        const FVector TraceStart = Candidate + FVector(0.f, 0.f, GroundTraceHeight);
        const FVector TraceEnd = Candidate - FVector(0.f, 0.f, GroundTraceDepth);

        FHitResult HitResult;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(ShijuArenaPlayerNearbyTimeRiftTrace), false);
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(TargetPlayer);

        if (ShijuBoss)
        {
            Params.AddIgnoredActor(ShijuBoss);
        }

        const bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            Params
        );

        if (!bHit)
        {
            continue;
        }

        OutLocation = HitResult.ImpactPoint + FVector(0.f, 0.f, 3.f);

        if (bDebugLog)
        {
            DrawDebugSphere(
                GetWorld(),
                OutLocation,
                45.f,
                16,
                FColor::Cyan,
                false,
                2.f
            );

            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[Shiju Arena] Resolved TimeRift near player: %s | Location = %s"),
                *TargetPlayer->GetName(),
                *OutLocation.ToString()
            );
        }

        return true;
    }

    return false;
}

bool AShijuArenaManager::ResolveRandomGroundLocationInArena(FVector& OutLocation) const
{
    if (!GetWorld())
    {
        return false;
    }

    const FVector ArenaCenter = GetActorLocation();

    for (int32 TryIndex = 0; TryIndex < 12; ++TryIndex)
    {
        const float Angle = FMath::FRandRange(0.f, 2.f * PI);
        const float Distance = FMath::Sqrt(FMath::FRand()) * SpawnRadius;

        const FVector Offset(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.f
        );

        const FVector Candidate = ArenaCenter + Offset;
        const FVector TraceStart = Candidate + FVector(0.f, 0.f, GroundTraceHeight);
        const FVector TraceEnd = Candidate - FVector(0.f, 0.f, GroundTraceDepth);

        FHitResult HitResult;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(ShijuArenaGroundTrace), false);
        Params.AddIgnoredActor(this);

        if (ShijuBoss)
        {
            Params.AddIgnoredActor(ShijuBoss);
        }

        const bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            Params
        );

        if (!bHit)
        {
            continue;
        }

        OutLocation = HitResult.ImpactPoint + FVector(0.f, 0.f, 3.f);

        if (bDebugLog)
        {
            DrawDebugSphere(
                GetWorld(),
                OutLocation,
                40.f,
                16,
                FColor::Cyan,
                false,
                2.f
            );
        }

        return true;
    }

    return false;
}

void AShijuArenaManager::CleanupInvalidRifts()
{
    ActiveRifts.RemoveAll(
        [](const TWeakObjectPtr<AShijuTimeRiftZone>& Rift)
        {
            return !Rift.IsValid();
        }
    );
}

void AShijuArenaManager::UpdateBellWaveSpawn()
{
    if (!GetWorld() || !ShijuBoss || !bEnableBellWave || !BellWaveActorClass)
    {
        return;
    }

    const int32 CurrentPhase = ShijuBoss->GetCurrentPhase();

    if (CurrentPhase < 3)
    {
        return;
    }

    if (ActiveBellWave.IsValid())
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    if ((CurrentTime - LastBellWaveSpawnTime) < Phase3BellWaveInterval)
    {
        return;
    }

    SpawnBellWave();
    LastBellWaveSpawnTime = CurrentTime;
}

void AShijuArenaManager::SpawnBellWave()
{
    if (!GetWorld() || !BellWaveActorClass)
    {
        return;
    }

    const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 5.f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AShijuBellWaveActor* BellWave = GetWorld()->SpawnActor<AShijuBellWaveActor>(
        BellWaveActorClass,
        SpawnLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (!BellWave)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Arena] Failed to spawn BellWave"));
        return;
    }

    BellWave->InitBellWave(
        ShijuBoss,
        BellWaveMaxRadius,
        BellWaveExpansionDuration,
        BellWaveDamage,
        bBellWaveAppliesTimeMark
    );

    ActiveBellWave = BellWave;

    if (bDebugLog)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Shiju Arena] Spawn BellWave | Radius = %.2f | Duration = %.2f"),
            BellWaveMaxRadius,
            BellWaveExpansionDuration
        );

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                4.f,
                FColor::Purple,
                FString::Printf(TEXT("[Shiju Arena] Spawn BellWave | Radius %.0f"), BellWaveMaxRadius)
            );
        }
    }
}