#include "ShijuBellTarget.h"

#include "../Shiju/ShijuBoss.h"
#include "ShijuTimeRiftZone.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

TMap<TWeakObjectPtr<ACharacter>, FShijuBellSlowState> AShijuBellTarget::GlobalSlowStates;

AShijuBellTarget::AShijuBellTarget()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    BellCollision = CreateDefaultSubobject<USphereComponent>(TEXT("BellCollision"));
    BellCollision->SetupAttachment(Root);
    BellCollision->InitSphereRadius(180.f);
    BellCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BellCollision->SetCollisionObjectType(ECC_WorldStatic);
    BellCollision->SetCollisionResponseToAllChannels(ECR_Block);
    BellCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    BellCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    BellCollision->SetGenerateOverlapEvents(false);

    PlayerBlockCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerBlockCollision"));
    PlayerBlockCollision->SetupAttachment(Root);
    PlayerBlockCollision->InitSphereRadius(110.f);
    PlayerBlockCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PlayerBlockCollision->SetCollisionObjectType(ECC_WorldStatic);
    PlayerBlockCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    PlayerBlockCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    PlayerBlockCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    PlayerBlockCollision->SetGenerateOverlapEvents(false);

    BellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BellMesh"));
    BellMesh->SetupAttachment(Root);
    BellMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    AimOffset = FVector(0.f, 0.f, 80.f);

    bOnlyTriggeredByShiju = true;
    TriggerCooldown = 3.f;
    bCoolingDown = false;

    EffectRadius = 0.f;

    Phase2EffectCandidates =
    {
        EShijuBellEffectType::Slow,
        EShijuBellEffectType::TimeMark,
        EShijuBellEffectType::PlayerNearbyTimeRift
    };

    Phase3EffectCandidates =
    {
        EShijuBellEffectType::Slow,
        EShijuBellEffectType::TimeMark,
        EShijuBellEffectType::Knockback,
        EShijuBellEffectType::PlayerNearbyTimeRift,
        EShijuBellEffectType::EmpowerNextBossArrow,
        EShijuBellEffectType::PositionRewind
    };

    SlowMultiplier = 0.5f;
    SlowDuration = 3.f;

    KnockbackStrength = 900.f;
    KnockbackUpwardStrength = 250.f;

    TimeRiftZoneClass = nullptr;
    TimeRiftMinDistanceFromPlayer = 300.f;
    TimeRiftMaxDistanceFromPlayer = 900.f;
    TimeRiftRadius = 300.f;
    TimeRiftDuration = 6.f;
    TimeRiftSlowMultiplier = 0.7f;
    GroundTraceHeight = 1000.f;
    GroundTraceDepth = 3000.f;

    NextArrowDamageMultiplier = 1.5f;
    NextArrowSpeedMultiplier = 1.2f;

    RewindSecondsBefore = 2.f;
    PositionHistoryMaxDuration = 4.f;
    PositionHistorySampleInterval = 0.1f;

    bDebugLog = true;
    bDrawEffectRadius = true;
}

void AShijuBellTarget::BeginPlay()
{
    Super::BeginPlay();

    if (bDebugLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] BeginPlay: %s"), *GetName());
    }
}

void AShijuBellTarget::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    RecordPlayerPositionHistory();
}

float AShijuBellTarget::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser
)
{
    const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (bCoolingDown)
    {
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] Ignored. CoolingDown: %s"), *GetName());
        }

        return AppliedDamage;
    }

    if (bOnlyTriggeredByShiju && !Cast<AShijuBoss>(DamageCauser))
    {
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] Ignored. DamageCauser is not ShijuBoss: %s"), *GetName());
        }

        return AppliedDamage;
    }

    TriggerRandomBellEffect(DamageCauser);

    bCoolingDown = true;

    if (GetWorld() && TriggerCooldown > 0.f)
    {
        GetWorldTimerManager().SetTimer(
            CooldownTimerHandle,
            this,
            &AShijuBellTarget::EndCooldown,
            TriggerCooldown,
            false
        );
    }

    return AppliedDamage;
}

FVector AShijuBellTarget::GetBellAimLocation() const
{
    return GetActorLocation() + AimOffset;
}

bool AShijuBellTarget::CanBeShot() const
{
    return !bCoolingDown;
}

void AShijuBellTarget::TriggerRandomBellEffect(AActor* DamageCauser)
{
    AShijuBoss* ShijuBoss = Cast<AShijuBoss>(DamageCauser);
    const int32 CurrentPhase = ShijuBoss ? ShijuBoss->GetCurrentPhase() : 2;

    if (CurrentPhase < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] Ignored. Phase < 2"));
        return;
    }

    TArray<ACharacter*> AffectedCharacters;
    GetAffectedPlayerCharacters(AffectedCharacters);

    if (bDrawEffectRadius && GetWorld() && EffectRadius > 0.f)
    {
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            EffectRadius,
            32,
            FColor::Yellow,
            false,
            2.f
        );
    }

    const EShijuBellEffectType SelectedEffect = ChooseEffectByPhase(CurrentPhase);

    switch (SelectedEffect)
    {
    case EShijuBellEffectType::Slow:
        ApplySlowEffect(AffectedCharacters);
        break;

    case EShijuBellEffectType::TimeMark:
        ApplyTimeMarkEffect(AffectedCharacters, DamageCauser);
        break;

    case EShijuBellEffectType::Knockback:
        ApplyKnockbackEffect(AffectedCharacters);
        break;

    case EShijuBellEffectType::PlayerNearbyTimeRift:
        ApplyPlayerNearbyTimeRiftEffect(AffectedCharacters);
        break;

    case EShijuBellEffectType::EmpowerNextBossArrow:
        ApplyEmpowerNextBossArrowEffect(DamageCauser);
        break;

    case EShijuBellEffectType::PositionRewind:
        ApplyPositionRewindEffect(AffectedCharacters);
        break;

    default:
        break;
    }

    const FString EffectName = GetEffectName(SelectedEffect);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Bell] Trigger Effect | Bell = %s | Phase = %d | Effect = %s | AffectedPlayers = %d"),
        *GetName(),
        CurrentPhase,
        *EffectName,
        AffectedCharacters.Num()
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Yellow,
            FString::Printf(TEXT("[Shiju Bell] %s | Phase %d"), *EffectName, CurrentPhase)
        );
    }
}

EShijuBellEffectType AShijuBellTarget::ChooseEffectByPhase(int32 CurrentPhase) const
{
    const TArray<EShijuBellEffectType>& Candidates =
        CurrentPhase >= 3 ? Phase3EffectCandidates : Phase2EffectCandidates;

    if (Candidates.Num() <= 0)
    {
        return EShijuBellEffectType::Slow;
    }

    return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

void AShijuBellTarget::GetAffectedPlayerCharacters(TArray<ACharacter*>& OutCharacters) const
{
    OutCharacters.Reset();

    if (!GetWorld())
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

    const FVector BellLocation = GetActorLocation();
    const float RadiusSquared = FMath::Square(EffectRadius);

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

        if (EffectRadius > 0.f)
        {
            const float DistSquared = FVector::DistSquared2D(Character->GetActorLocation(), BellLocation);
            if (DistSquared > RadiusSquared)
            {
                continue;
            }
        }

        OutCharacters.Add(Character);
    }
}

void AShijuBellTarget::ApplySlowEffect(const TArray<ACharacter*>& Characters)
{
    for (ACharacter* Character : Characters)
    {
        ApplySlowToCharacter(Character);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] Slow Effect | Count = %d"), Characters.Num());
}

void AShijuBellTarget::ApplyTimeMarkEffect(const TArray<ACharacter*>& Characters, AActor* DamageCauser)
{
    AShijuBoss* ShijuBoss = Cast<AShijuBoss>(DamageCauser);
    if (!ShijuBoss)
    {
        return;
    }

    for (ACharacter* Character : Characters)
    {
        if (!Character)
        {
            continue;
        }

        ShijuBoss->RegisterTimeMarkHit();
    }

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] TimeMark Effect | Count = %d"), Characters.Num());
}

void AShijuBellTarget::ApplyKnockbackEffect(const TArray<ACharacter*>& Characters)
{
    const FVector BellLocation = GetActorLocation();

    for (ACharacter* Character : Characters)
    {
        if (!Character)
        {
            continue;
        }

        FVector Direction = Character->GetActorLocation() - BellLocation;
        Direction.Z = 0.f;
        Direction = Direction.GetSafeNormal();

        if (Direction.IsNearlyZero())
        {
            Direction = Character->GetActorForwardVector();
        }

        const FVector LaunchVelocity =
            Direction * KnockbackStrength +
            FVector(0.f, 0.f, KnockbackUpwardStrength);

        Character->LaunchCharacter(LaunchVelocity, true, true);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] Knockback Effect | Count = %d"), Characters.Num());
}

void AShijuBellTarget::ApplyPlayerNearbyTimeRiftEffect(const TArray<ACharacter*>& Characters)
{
    if (Characters.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] TimeRift Effect Failed. No player"));
        return;
    }

    ACharacter* TargetCharacter = Characters[FMath::RandRange(0, Characters.Num() - 1)];
    if (!TargetCharacter)
    {
        return;
    }

    const bool bSpawned = SpawnTimeRiftNearCharacter(TargetCharacter);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Bell] PlayerNearbyTimeRift Effect | Target = %s | Spawned = %s"),
        *TargetCharacter->GetName(),
        bSpawned ? TEXT("true") : TEXT("false")
    );
}

void AShijuBellTarget::ApplyEmpowerNextBossArrowEffect(AActor* DamageCauser)
{
    AShijuBoss* ShijuBoss = Cast<AShijuBoss>(DamageCauser);
    if (!ShijuBoss)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] Empower Failed. DamageCauser is not ShijuBoss"));
        return;
    }

    ShijuBoss->EmpowerNextArrow(NextArrowDamageMultiplier, NextArrowSpeedMultiplier);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Bell] EmpowerNextBossArrow Effect | Damage x%.2f | Speed x%.2f"),
        NextArrowDamageMultiplier,
        NextArrowSpeedMultiplier
    );
}

void AShijuBellTarget::ApplyPositionRewindEffect(const TArray<ACharacter*>& Characters)
{
    int32 RewindCount = 0;

    for (ACharacter* Character : Characters)
    {
        if (!Character)
        {
            continue;
        }

        FVector RewindLocation = FVector::ZeroVector;
        FRotator RewindRotation = FRotator::ZeroRotator;

        if (!FindRewindTransform(Character, RewindLocation, RewindRotation))
        {
            continue;
        }

        Character->SetActorLocationAndRotation(
            RewindLocation,
            RewindRotation,
            false,
            nullptr,
            ETeleportType::TeleportPhysics
        );

        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->Velocity = FVector::ZeroVector;
        }

        ++RewindCount;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] PositionRewind Effect | Count = %d"), RewindCount);
}

void AShijuBellTarget::ApplySlowToCharacter(ACharacter* Character)
{
    if (!Character || !GetWorld())
    {
        return;
    }

    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }

    TWeakObjectPtr<ACharacter> CharacterKey(Character);

    FShijuBellSlowState* ExistingState = GlobalSlowStates.Find(CharacterKey);
    if (ExistingState)
    {
        ++ExistingState->ActiveSlowCount;
        MovementComp->MaxWalkSpeed = ExistingState->OriginalMaxWalkSpeed * SlowMultiplier;
    }
    else
    {
        FShijuBellSlowState NewState;
        NewState.OriginalMaxWalkSpeed = MovementComp->MaxWalkSpeed;
        NewState.ActiveSlowCount = 1;

        GlobalSlowStates.Add(CharacterKey, NewState);

        MovementComp->MaxWalkSpeed = NewState.OriginalMaxWalkSpeed * SlowMultiplier;
    }

    FTimerHandle RestoreHandle;
    FTimerDelegate RestoreDelegate = FTimerDelegate::CreateUObject(
        this,
        &AShijuBellTarget::RestoreSlowFromCharacter,
        Character
    );

    GetWorldTimerManager().SetTimer(
        RestoreHandle,
        RestoreDelegate,
        SlowDuration,
        false
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Bell] Apply Slow: %s | Speed = %.2f"),
        *Character->GetName(),
        MovementComp->MaxWalkSpeed
    );
}

void AShijuBellTarget::RestoreSlowFromCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }

    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }

    TWeakObjectPtr<ACharacter> CharacterKey(Character);

    FShijuBellSlowState* ExistingState = GlobalSlowStates.Find(CharacterKey);
    if (!ExistingState)
    {
        return;
    }

    --ExistingState->ActiveSlowCount;

    if (ExistingState->ActiveSlowCount > 0)
    {
        MovementComp->MaxWalkSpeed = ExistingState->OriginalMaxWalkSpeed * SlowMultiplier;
        return;
    }

    MovementComp->MaxWalkSpeed = ExistingState->OriginalMaxWalkSpeed;
    GlobalSlowStates.Remove(CharacterKey);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju Bell] Restore Slow: %s | Speed = %.2f"),
        *Character->GetName(),
        MovementComp->MaxWalkSpeed
    );
}

bool AShijuBellTarget::SpawnTimeRiftNearCharacter(ACharacter* Character)
{
    if (!GetWorld() || !Character || !TimeRiftZoneClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] TimeRift spawn failed. Class or Character is null"));
        return false;
    }

    FVector SpawnLocation = FVector::ZeroVector;
    if (!ResolveRandomGroundLocationNearCharacter(Character, SpawnLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] TimeRift spawn failed. No ground location"));
        return false;
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
        return false;
    }

    TimeRift->InitTimeRift(
        TimeRiftRadius,
        TimeRiftDuration,
        TimeRiftSlowMultiplier
    );

    return true;
}

bool AShijuBellTarget::ResolveRandomGroundLocationNearCharacter(ACharacter* Character, FVector& OutLocation) const
{
    if (!GetWorld() || !Character)
    {
        return false;
    }

    const FVector PlayerLocation = Character->GetActorLocation();

    for (int32 TryIndex = 0; TryIndex < 12; ++TryIndex)
    {
        const float Angle = FMath::FRandRange(0.f, 2.f * PI);
        const float Distance = FMath::FRandRange(TimeRiftMinDistanceFromPlayer, TimeRiftMaxDistanceFromPlayer);

        const FVector Offset(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.f
        );

        const FVector Candidate = PlayerLocation + Offset;
        const FVector TraceStart = Candidate + FVector(0.f, 0.f, GroundTraceHeight);
        const FVector TraceEnd = Candidate - FVector(0.f, 0.f, GroundTraceDepth);

        FHitResult HitResult;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(ShijuBellTimeRiftGroundTrace), false);
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Character);

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
                50.f,
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

void AShijuBellTarget::RecordPlayerPositionHistory()
{
    if (!GetWorld())
    {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (!Character || !Character->IsPlayerControlled())
        {
            continue;
        }

        TWeakObjectPtr<ACharacter> CharacterKey(Character);
        FShijuPlayerPositionHistory& History = PlayerPositionHistories.FindOrAdd(CharacterKey);

        if ((CurrentTime - History.LastSampleTime) < PositionHistorySampleInterval)
        {
            continue;
        }

        History.LastSampleTime = CurrentTime;

        FShijuPlayerPositionSample NewSample;
        NewSample.TimeSeconds = CurrentTime;
        NewSample.Location = Character->GetActorLocation();
        NewSample.Rotation = Character->GetActorRotation();

        History.Samples.Add(NewSample);

        const float OldestAllowedTime = CurrentTime - PositionHistoryMaxDuration;

        History.Samples.RemoveAll(
            [OldestAllowedTime](const FShijuPlayerPositionSample& Sample)
            {
                return Sample.TimeSeconds < OldestAllowedTime;
            }
        );
    }
}

bool AShijuBellTarget::FindRewindTransform(ACharacter* Character, FVector& OutLocation, FRotator& OutRotation) const
{
    if (!Character || !GetWorld())
    {
        return false;
    }

    const TWeakObjectPtr<ACharacter> CharacterKey(Character);
    const FShijuPlayerPositionHistory* History = PlayerPositionHistories.Find(CharacterKey);
    if (!History || History->Samples.Num() <= 0)
    {
        return false;
    }

    const float TargetTime = GetWorld()->GetTimeSeconds() - RewindSecondsBefore;

    const FShijuPlayerPositionSample* BestSample = nullptr;

    for (const FShijuPlayerPositionSample& Sample : History->Samples)
    {
        if (Sample.TimeSeconds <= TargetTime)
        {
            BestSample = &Sample;
        }
        else
        {
            break;
        }
    }

    if (!BestSample)
    {
        BestSample = &History->Samples[0];
    }

    OutLocation = BestSample->Location;
    OutRotation = BestSample->Rotation;
    return true;
}

void AShijuBellTarget::EndCooldown()
{
    bCoolingDown = false;

    UE_LOG(LogTemp, Warning, TEXT("[Shiju Bell] Cooldown End: %s"), *GetName());
}

FString AShijuBellTarget::GetEffectName(EShijuBellEffectType EffectType) const
{
    switch (EffectType)
    {
    case EShijuBellEffectType::Slow:
        return TEXT("Slow");

    case EShijuBellEffectType::TimeMark:
        return TEXT("TimeMark");

    case EShijuBellEffectType::Knockback:
        return TEXT("Knockback");

    case EShijuBellEffectType::PlayerNearbyTimeRift:
        return TEXT("PlayerNearbyTimeRift");

    case EShijuBellEffectType::EmpowerNextBossArrow:
        return TEXT("EmpowerNextBossArrow");

    case EShijuBellEffectType::PositionRewind:
        return TEXT("PositionRewind");

    default:
        return TEXT("Unknown");
    }
}