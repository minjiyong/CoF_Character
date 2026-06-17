#include "ShijuQArea.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

AShijuQArea::AShijuQArea()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetupAttachment(Root);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    AreaSphere->SetSphereRadius(200.f);

    ImpactParticleAsset = nullptr;
    ImpactParticleScale = FVector(1.f, 1.f, 1.f);
    WarningFXScale = FVector(1.f, 1.f, 1.f);
    MagicCircleScale = FVector(1.f, 1.f, 1.f);

    DamagePerTick = 10.f;
    BurnDuration = 3.f;
    BurnTickInterval = 0.5f;
    DamageCauserActor = nullptr;

    WarningFXComp = nullptr;
    MagicCircleComp = nullptr;
    bBurnActive = false;

    InitialLifeSpan = 0.f;
}

void AShijuQArea::BeginPlay()
{
    Super::BeginPlay();
    CacheEffectComponents();
}

void AShijuQArea::CacheEffectComponents()
{
    if (!WarningFXComp)
    {
        TArray<UNiagaraComponent*> NiagaraComponents;
        GetComponents(NiagaraComponents);

        for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
        {
            if (!NiagaraComp)
            {
                continue;
            }

            const FString CompName = NiagaraComp->GetName();
            if (CompName.Contains(TEXT("NS_Shiju_Q_Warning")) || CompName.Contains(TEXT("Warning")))
            {
                WarningFXComp = NiagaraComp;
                break;
            }
        }
    }

    if (!MagicCircleComp)
    {
        TArray<UParticleSystemComponent*> ParticleComponents;
        GetComponents(ParticleComponents);

        for (UParticleSystemComponent* PSC : ParticleComponents)
        {
            if (!PSC)
            {
                continue;
            }

            const FString CompName = PSC->GetName();
            if (CompName.Contains(TEXT("PS_MagicCircle")) || CompName.Contains(TEXT("MagicCircle")))
            {
                MagicCircleComp = PSC;
                break;
            }
        }
    }

    if (WarningFXComp)
    {
        WarningFXComp->SetVisibility(false);
        WarningFXComp->Deactivate();
    }

    if (MagicCircleComp)
    {
        MagicCircleComp->SetVisibility(false);
        MagicCircleComp->DeactivateSystem();
    }
}

void AShijuQArea::InitWarning(
    float InRadius,
    float InDamagePerTick,
    float InDuration,
    float InTickInterval,
    AActor* InDamageCauser
)
{
    DamagePerTick = InDamagePerTick;
    BurnDuration = InDuration;
    BurnTickInterval = InTickInterval;
    DamageCauserActor = InDamageCauser;

    if (AreaSphere)
    {
        AreaSphere->SetSphereRadius(InRadius);
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    CacheEffectComponents();

    if (WarningFXComp)
    {
        WarningFXComp->SetVisibility(true);
        WarningFXComp->Activate(true);
    }

    if (MagicCircleComp)
    {
        MagicCircleComp->SetVisibility(false);
        MagicCircleComp->DeactivateSystem();
    }

    bBurnActive = false;
}

void AShijuQArea::ActivateBurnField(const FVector& ImpactLocation)
{
    SetActorLocation(ImpactLocation + FVector(0.f, 0.f, 2.f));

    CacheEffectComponents();

    if (WarningFXComp)
    {
        WarningFXComp->Deactivate();
        WarningFXComp->SetVisibility(false);
    }

    if (ImpactParticleAsset)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ImpactParticleAsset,
            FTransform(FRotator::ZeroRotator, GetActorLocation(), ImpactParticleScale),
            true
        );
    }

    if (MagicCircleComp)
    {
        MagicCircleComp->SetVisibility(true);
        MagicCircleComp->ActivateSystem(true);
    }

    if (AreaSphere)
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    bBurnActive = true;

    ApplyBurnDamage();

    GetWorldTimerManager().SetTimer(
        BurnTickHandle,
        this,
        &AShijuQArea::ApplyBurnDamage,
        BurnTickInterval,
        true
    );

    GetWorldTimerManager().SetTimer(
        BurnEndHandle,
        this,
        &AShijuQArea::EndBurnField,
        BurnDuration,
        false
    );
}

void AShijuQArea::ApplyBurnDamage()
{
    if (!bBurnActive || !AreaSphere)
    {
        return;
    }

    TArray<AActor*> OverlappingActors;
    AreaSphere->GetOverlappingActors(OverlappingActors);

    AController* InstigatorController = nullptr;
    if (APawn* OwnerPawn = Cast<APawn>(DamageCauserActor))
    {
        InstigatorController = OwnerPawn->GetController();
    }

    for (AActor* OtherActor : OverlappingActors)
    {
        if (!OtherActor || OtherActor == this || OtherActor == DamageCauserActor)
        {
            continue;
        }

        UGameplayStatics::ApplyDamage(
            OtherActor,
            DamagePerTick,
            InstigatorController,
            DamageCauserActor,
            nullptr
        );
    }
}

void AShijuQArea::EndBurnField()
{
    bBurnActive = false;

    GetWorldTimerManager().ClearTimer(BurnTickHandle);

    if (AreaSphere)
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (MagicCircleComp)
    {
        MagicCircleComp->DeactivateSystem();
        MagicCircleComp->SetVisibility(false);
    }

    Destroy();
}