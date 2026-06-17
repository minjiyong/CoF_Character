#include "ShijuBellWaveActor.h"

#include "../Shiju/ShijuBoss.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

AShijuBellWaveActor::AShijuBellWaveActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    WaveSphere = CreateDefaultSubobject<USphereComponent>(TEXT("WaveSphere"));
    WaveSphere->SetupAttachment(Root);
    WaveSphere->InitSphereRadius(1.f);
    WaveSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WaveSphere->SetGenerateOverlapEvents(false);

    WaveFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WaveFX"));
    WaveFX->SetupAttachment(Root);
    WaveFX->SetAutoActivate(true);

    MaxRadius = 1800.f;
    ExpansionDuration = 2.0f;
    WaveThickness = 120.f;
    Damage = 0.f;
    bApplyTimeMark = true;
    bAffectOnlyPlayerControlled = true;
    bDrawDebugWave = true;

    CurrentRadius = 0.f;
    ElapsedTime = 0.f;

    ShijuBoss = nullptr;

    InitialLifeSpan = 0.f;
}

void AShijuBellWaveActor::BeginPlay()
{
    Super::BeginPlay();

    CurrentRadius = 0.f;
    ElapsedTime = 0.f;

    if (WaveSphere)
    {
        WaveSphere->SetSphereRadius(1.f);
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju BellWave] Begin | MaxRadius = %.2f, Duration = %.2f, Thickness = %.2f"),
        MaxRadius,
        ExpansionDuration,
        WaveThickness
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Purple,
            FString::Printf(TEXT("[Shiju BellWave] Begin | Radius %.0f"), MaxRadius)
        );
    }
}

void AShijuBellWaveActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    ElapsedTime += DeltaSeconds;

    const float Alpha = ExpansionDuration > KINDA_SMALL_NUMBER
        ? FMath::Clamp(ElapsedTime / ExpansionDuration, 0.f, 1.f)
        : 1.f;

    CurrentRadius = MaxRadius * Alpha;

    if (WaveSphere)
    {
        WaveSphere->SetSphereRadius(FMath::Max(1.f, CurrentRadius), true);
    }

    if (bDrawDebugWave && GetWorld())
    {
        const float HalfThickness = WaveThickness * 0.5f;
        const float InnerRadius = FMath::Max(0.f, CurrentRadius - HalfThickness);
        const float OuterRadius = CurrentRadius + HalfThickness;

        // 실제 판정 중심선
        DrawDebugCircle(
            GetWorld(),
            GetActorLocation() + FVector(0.f, 0.f, 8.f),
            CurrentRadius,
            96,
            FColor::Purple,
            false,
            0.f,
            0,
            6.f,
            FVector(1.f, 0.f, 0.f),
            FVector(0.f, 1.f, 0.f),
            false
        );

        // 판정 안쪽 경계
        DrawDebugCircle(
            GetWorld(),
            GetActorLocation() + FVector(0.f, 0.f, 6.f),
            InnerRadius,
            96,
            FColor::Magenta,
            false,
            0.f,
            0,
            2.f,
            FVector(1.f, 0.f, 0.f),
            FVector(0.f, 1.f, 0.f),
            false
        );

        // 판정 바깥쪽 경계
        DrawDebugCircle(
            GetWorld(),
            GetActorLocation() + FVector(0.f, 0.f, 6.f),
            OuterRadius,
            96,
            FColor::Magenta,
            false,
            0.f,
            0,
            2.f,
            FVector(1.f, 0.f, 0.f),
            FVector(0.f, 1.f, 0.f),
            false
        );
    }

    ScanCharactersInWave();

    if (Alpha >= 1.f)
    {
        EndBellWave();
    }
}

void AShijuBellWaveActor::InitBellWave(
    AShijuBoss* InShijuBoss,
    float InMaxRadius,
    float InExpansionDuration,
    float InDamage,
    bool bInApplyTimeMark
)
{
    ShijuBoss = InShijuBoss;
    MaxRadius = FMath::Max(1.f, InMaxRadius);
    ExpansionDuration = FMath::Max(0.01f, InExpansionDuration);
    Damage = FMath::Max(0.f, InDamage);
    bApplyTimeMark = bInApplyTimeMark;

    CurrentRadius = 0.f;
    ElapsedTime = 0.f;
    HitCharacters.Empty();

    if (WaveSphere)
    {
        WaveSphere->SetSphereRadius(1.f);
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju BellWave] Init | MaxRadius = %.2f, Duration = %.2f, Thickness = %.2f, Damage = %.2f, Mark = %s"),
        MaxRadius,
        ExpansionDuration,
        WaveThickness,
        Damage,
        bApplyTimeMark ? TEXT("true") : TEXT("false")
    );
}

void AShijuBellWaveActor::ScanCharactersInWave()
{
    if (!GetWorld() || CurrentRadius <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundCharacters);

    const FVector WaveCenter = GetActorLocation();

    const float HalfThickness = WaveThickness * 0.5f;
    const float InnerRadius = FMath::Max(0.f, CurrentRadius - HalfThickness);
    const float OuterRadius = CurrentRadius + HalfThickness;

    const float InnerRadiusSquared = FMath::Square(InnerRadius);
    const float OuterRadiusSquared = FMath::Square(OuterRadius);

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

        TWeakObjectPtr<ACharacter> CharacterKey(Character);
        if (HitCharacters.Contains(CharacterKey))
        {
            continue;
        }

        const float DistanceSquared2D = FVector::DistSquared2D(Character->GetActorLocation(), WaveCenter);

        // 도넛형 판정:
        // 현재 충격파 반경 주변의 얇은 링 안에 있을 때만 맞는다.
        if (DistanceSquared2D < InnerRadiusSquared || DistanceSquared2D > OuterRadiusSquared)
        {
            continue;
        }

        HandleWaveHit(Character);
    }
}

void AShijuBellWaveActor::HandleWaveHit(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }

    TWeakObjectPtr<ACharacter> CharacterKey(Character);
    HitCharacters.Add(CharacterKey);

    AController* InstigatorController = nullptr;
    if (APawn* BossPawn = Cast<APawn>(ShijuBoss))
    {
        InstigatorController = BossPawn->GetController();
    }

    if (Damage > 0.f)
    {
        UGameplayStatics::ApplyDamage(
            Character,
            Damage,
            InstigatorController,
            ShijuBoss,
            nullptr
        );
    }

    if (bApplyTimeMark && ShijuBoss)
    {
        ShijuBoss->RegisterTimeMarkHit();
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Shiju BellWave] Hit: %s | Radius = %.2f | Damage = %.2f | ApplyMark = %s"),
        *Character->GetName(),
        CurrentRadius,
        Damage,
        bApplyTimeMark ? TEXT("true") : TEXT("false")
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.f,
            FColor::Purple,
            FString::Printf(TEXT("[BellWave] Hit %s"), *Character->GetName())
        );
    }
}

void AShijuBellWaveActor::EndBellWave()
{
    UE_LOG(LogTemp, Warning, TEXT("[Shiju BellWave] End"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.f,
            FColor::Purple,
            TEXT("[Shiju BellWave] End")
        );
    }

    Destroy();
}