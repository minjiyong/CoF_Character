#include "Projectiles/Kallari_Skill2A_ShurikenProjectile.h"

#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Skills/Kallari/Kallari_Skill2A_ShurikenTeleport.h"
#include "TP_Character.h"

AKallari_Skill2A_ShurikenProjectile::AKallari_Skill2A_ShurikenProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);

    Collision->InitSphereRadius(18.f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->InitialSpeed = 2600.f;
    ProjectileMovement->MaxSpeed = 2600.f;
    ProjectileMovement->bShouldBounce = false;
}

void AKallari_Skill2A_ShurikenProjectile::InitProjectile(
    ATP_Character* InOwnerCharacter,
    UCombatComponent* InCombatComp,
    UKallari_Skill2A_ShurikenTeleport* InOwningSkill,
    float InDamage,
    float InInitialSpeed,
    float InLifeSeconds,
    float InRadius)
{
    OwnerCharacter = InOwnerCharacter;
    OwningCombatComp = InCombatComp;
    OwningSkill = InOwningSkill;
    Damage = InDamage;

    SetOwner(InOwnerCharacter);
    SetInstigator(InOwnerCharacter);

    Collision->SetSphereRadius(InRadius);
    Collision->IgnoreActorWhenMoving(InOwnerCharacter, true);

    ProjectileMovement->InitialSpeed = InInitialSpeed;
    ProjectileMovement->MaxSpeed = InInitialSpeed;
    ProjectileMovement->Velocity = GetActorForwardVector() * InInitialSpeed;

    SetLifeSpan(InLifeSeconds);
}

void AKallari_Skill2A_ShurikenProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (Collision)
    {
        Collision->OnComponentBeginOverlap.AddDynamic(this, &AKallari_Skill2A_ShurikenProjectile::HandleOverlap);
    }

    if (ProjectileMovement)
    {
        ProjectileMovement->OnProjectileStop.AddDynamic(this, &AKallari_Skill2A_ShurikenProjectile::HandleProjectileStop);
    }
}

void AKallari_Skill2A_ShurikenProjectile::LifeSpanExpired()
{
    if (!bResolved)
    {
        ResolveAndDestroy(GetActorLocation(), FVector::UpVector);
        return;
    }

    Super::LifeSpanExpired();
}

void AKallari_Skill2A_ShurikenProjectile::HandleOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (bResolved) return;
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner()) return;

    if (OwningCombatComp.IsValid())
    {
        FVector HitPoint = FVector(SweepResult.ImpactPoint);
        if (HitPoint.IsNearlyZero())
        {
            HitPoint = OtherActor->GetActorLocation();
        }

        FVector HitNormal = FVector(SweepResult.ImpactNormal);
        if (HitNormal.IsNearlyZero())
        {
            HitNormal = -GetActorForwardVector();
        }

        OwningCombatComp->ApplyHitToActor(OtherActor, Damage, HitPoint, HitNormal);
        ResolveAndDestroy(HitPoint, HitNormal);
    }
}

void AKallari_Skill2A_ShurikenProjectile::HandleProjectileStop(const FHitResult& ImpactResult)
{
    if (bResolved) return;

    FVector MarkLocation = FVector(ImpactResult.ImpactPoint);
    if (MarkLocation.IsNearlyZero())
    {
        MarkLocation = GetActorLocation();
    }

    FVector MarkNormal = FVector(ImpactResult.ImpactNormal);
    if (MarkNormal.IsNearlyZero())
    {
        MarkNormal = FVector::UpVector;
    }

    ResolveAndDestroy(MarkLocation, MarkNormal);
}

void AKallari_Skill2A_ShurikenProjectile::ResolveAndDestroy(const FVector& MarkLocation, const FVector& MarkNormal)
{
    if (bResolved) return;
    bResolved = true;

    if (OwningSkill.IsValid())
    {
        OwningSkill->OnProjectileResolved(MarkLocation, MarkNormal);
    }

    Destroy();
}