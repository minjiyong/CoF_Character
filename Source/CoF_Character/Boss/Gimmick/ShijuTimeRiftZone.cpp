#include "ShijuTimeRiftZone.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"

TMap<TWeakObjectPtr<ACharacter>, FShijuTimeRiftGlobalAffectedState> AShijuTimeRiftZone::GlobalAffectedCharacters;

AShijuTimeRiftZone::AShijuTimeRiftZone()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    RiftSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RiftSphere"));
    RiftSphere->SetupAttachment(Root);
    RiftSphere->InitSphereRadius(300.f);
    RiftSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    RiftSphere->SetCollisionObjectType(ECC_WorldDynamic);
    RiftSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    RiftSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    RiftSphere->SetGenerateOverlapEvents(true);

    RiftFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RiftFX"));
    RiftFX->SetupAttachment(Root);
    RiftFX->SetAutoActivate(true);

    Radius = 300.f;
    Duration = 6.f;
    SlowMultiplier = 0.7f;
    bAutoDestroyAfterDuration = true;
    bAffectOnlyPlayerControlled = true;
    bDrawDebugSphere = true;
    ScanInterval = 0.1f;

    InitialLifeSpan = 0.f;
}

void AShijuTimeRiftZone::BeginPlay()
{
    Super::BeginPlay();

    if (RiftSphere)
    {
        RiftSphere->SetSphereRadius(Radius);
        RiftSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        RiftSphere->SetCollisionObjectType(ECC_WorldDynamic);
        RiftSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        RiftSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        RiftSphere->SetGenerateOverlapEvents(true);

        RiftSphere->OnComponentBeginOverlap.AddDynamic(this, &AShijuTimeRiftZone::OnRiftBeginOverlap);
        RiftSphere->OnComponentEndOverlap.AddDynamic(this, &AShijuTimeRiftZone::OnRiftEndOverlap);
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju TimeRift] BeginPlay | Radius = %.2f, Duration = %.2f, SlowMultiplier = %.2f"),
        Radius,
        Duration,
        SlowMultiplier
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Cyan,
            FString::Printf(TEXT("[Shiju TimeRift] Begin | Radius %.0f"), Radius)
        );
    }

    if (GetWorld())
    {
        GetWorldTimerManager().SetTimer(
            ScanTimerHandle,
            this,
            &AShijuTimeRiftZone::RefreshAffectedCharacters,
            ScanInterval,
            true
        );
    }

    if (bAutoDestroyAfterDuration && Duration > 0.f)
    {
        GetWorldTimerManager().SetTimer(
            EndTimerHandle,
            this,
            &AShijuTimeRiftZone::EndTimeRift,
            Duration,
            false
        );
    }

    RefreshAffectedCharacters();
}

void AShijuTimeRiftZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(ScanTimerHandle);
    GetWorldTimerManager().ClearTimer(EndTimerHandle);

    RestoreAllLocallyAffectedCharacters();

    Super::EndPlay(EndPlayReason);
}

void AShijuTimeRiftZone::InitTimeRift(float InRadius, float InDuration, float InSlowMultiplier)
{
    Radius = FMath::Max(1.f, InRadius);
    Duration = FMath::Max(0.f, InDuration);
    SlowMultiplier = FMath::Clamp(InSlowMultiplier, 0.01f, 1.f);

    if (RiftSphere)
    {
        RiftSphere->SetSphereRadius(Radius);
    }

    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(EndTimerHandle);

        if (bAutoDestroyAfterDuration && Duration > 0.f)
        {
            GetWorldTimerManager().SetTimer(
                EndTimerHandle,
                this,
                &AShijuTimeRiftZone::EndTimeRift,
                Duration,
                false
            );
        }
    }

    RefreshAffectedCharacters();
}

void AShijuTimeRiftZone::OnRiftBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!OtherActor)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Shiju TimeRift] Overlap Begin Event: %s"), *OtherActor->GetName());
}

void AShijuTimeRiftZone::OnRiftEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    if (!OtherActor)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Shiju TimeRift] Overlap End Event: %s"), *OtherActor->GetName());
}

void AShijuTimeRiftZone::RefreshAffectedCharacters()
{
    if (!GetWorld())
    {
        return;
    }

    if (bDrawDebugSphere)
    {
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            Radius,
            32,
            FColor::Cyan,
            false,
            ScanInterval
        );
    }

    TSet<TWeakObjectPtr<ACharacter>> CurrentCharactersInRift;

    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundCharacters);

    const FVector RiftLocation = GetActorLocation();
    const float RadiusSquared = FMath::Square(Radius);

    for (AActor* Actor : FoundCharacters)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (!Character)
        {
            continue;
        }

        if (bAffectOnlyPlayerControlled && !Character->IsPlayerControlled())
        {
            continue;
        }

        const float DistanceSquared2D = FVector::DistSquared2D(Character->GetActorLocation(), RiftLocation);
        if (DistanceSquared2D > RadiusSquared)
        {
            continue;
        }

        TWeakObjectPtr<ACharacter> CharacterKey(Character);
        CurrentCharactersInRift.Add(CharacterKey);

        if (!LocallyAffectedCharacters.Contains(CharacterKey))
        {
            ApplySlow(Character);
        }
    }

    TArray<ACharacter*> CharactersToRestore;

    for (const TWeakObjectPtr<ACharacter>& CharacterKey : LocallyAffectedCharacters)
    {
        ACharacter* Character = CharacterKey.Get();
        if (!Character)
        {
            continue;
        }

        if (!CurrentCharactersInRift.Contains(CharacterKey))
        {
            CharactersToRestore.Add(Character);
        }
    }

    for (ACharacter* Character : CharactersToRestore)
    {
        RestoreSlow(Character);
    }
}

void AShijuTimeRiftZone::ApplySlow(ACharacter* Character)
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

    if (LocallyAffectedCharacters.Contains(CharacterKey))
    {
        return;
    }

    LocallyAffectedCharacters.Add(CharacterKey);

    FShijuTimeRiftGlobalAffectedState* ExistingState = GlobalAffectedCharacters.Find(CharacterKey);
    if (ExistingState)
    {
        ++ExistingState->ActiveRiftCount;

        MovementComp->MaxWalkSpeed = ExistingState->OriginalMaxWalkSpeed * SlowMultiplier;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Shiju TimeRift] Add Rift Stack: %s, Stack = %d, Speed = %.2f"),
            *Character->GetName(),
            ExistingState->ActiveRiftCount,
            MovementComp->MaxWalkSpeed
        );

        return;
    }

    FShijuTimeRiftGlobalAffectedState NewState;
    NewState.OriginalMaxWalkSpeed = MovementComp->MaxWalkSpeed;
    NewState.ActiveRiftCount = 1;

    GlobalAffectedCharacters.Add(CharacterKey, NewState);

    MovementComp->MaxWalkSpeed = NewState.OriginalMaxWalkSpeed * SlowMultiplier;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju TimeRift] Apply Slow: %s, Speed %.2f -> %.2f"),
        *Character->GetName(),
        NewState.OriginalMaxWalkSpeed,
        MovementComp->MaxWalkSpeed
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Yellow,
            FString::Printf(
                TEXT("[TimeRift] Slow %s | %.0f -> %.0f"),
                *Character->GetName(),
                NewState.OriginalMaxWalkSpeed,
                MovementComp->MaxWalkSpeed
            )
        );
    }
}

void AShijuTimeRiftZone::RestoreSlow(ACharacter* Character)
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

    if (!LocallyAffectedCharacters.Contains(CharacterKey))
    {
        return;
    }

    LocallyAffectedCharacters.Remove(CharacterKey);

    FShijuTimeRiftGlobalAffectedState* ExistingState = GlobalAffectedCharacters.Find(CharacterKey);
    if (!ExistingState)
    {
        return;
    }

    --ExistingState->ActiveRiftCount;

    if (ExistingState->ActiveRiftCount > 0)
    {
        MovementComp->MaxWalkSpeed = ExistingState->OriginalMaxWalkSpeed * SlowMultiplier;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Shiju TimeRift] Remove Rift Stack: %s, Remaining Stack = %d, Speed = %.2f"),
            *Character->GetName(),
            ExistingState->ActiveRiftCount,
            MovementComp->MaxWalkSpeed
        );

        return;
    }

    MovementComp->MaxWalkSpeed = ExistingState->OriginalMaxWalkSpeed;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju TimeRift] Restore Speed: %s, Speed = %.2f"),
        *Character->GetName(),
        MovementComp->MaxWalkSpeed
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Yellow,
            FString::Printf(
                TEXT("[TimeRift] Restore %s | %.0f"),
                *Character->GetName(),
                MovementComp->MaxWalkSpeed
            )
        );
    }

    GlobalAffectedCharacters.Remove(CharacterKey);
}

void AShijuTimeRiftZone::RestoreAllLocallyAffectedCharacters()
{
    TArray<ACharacter*> CharactersToRestore;

    for (const TWeakObjectPtr<ACharacter>& CharacterKey : LocallyAffectedCharacters)
    {
        ACharacter* Character = CharacterKey.Get();
        if (!Character)
        {
            continue;
        }

        CharactersToRestore.Add(Character);
    }

    for (ACharacter* Character : CharactersToRestore)
    {
        RestoreSlow(Character);
    }

    LocallyAffectedCharacters.Empty();
}

void AShijuTimeRiftZone::EndTimeRift()
{
    UE_LOG(LogTemp, Warning, TEXT("[Shiju TimeRift] EndTimeRift"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Cyan,
            TEXT("[Shiju TimeRift] End")
        );
    }

    RestoreAllLocallyAffectedCharacters();
    Destroy();
}