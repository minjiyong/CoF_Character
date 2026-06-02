#include "Projectiles/Kallari_Skill2A_ShurikenProjectile.h"

#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Skills/Kallari/Kallari_Skill2A_ShurikenTeleport.h"
#include "Skills/Kallari/Kallari_Skill2B_ShurikenExplosion.h"
#include "Skills/Gideon/Gideon_Skill1B_WaterBomb.h"
#include "TP_Character.h"

#include "DrawDebugHelpers.h"

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
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
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
    UObject* InOwningSkill,
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

    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->InitialSpeed = InInitialSpeed;
    ProjectileMovement->MaxSpeed = InInitialSpeed;
    ProjectileMovement->Velocity = GetActorForwardVector() * InInitialSpeed;

    SetLifeSpan(InLifeSeconds);

#if !(UE_BUILD_SHIPPING)
    if (UWorld* World = GetWorld())
    {
        DrawDebugSphere(World, GetActorLocation(), InRadius, 16, FColor::Green, false, 2.0f, 0, 1.5f);
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("[Projectile Init] Damage=%.1f Speed=%.1f Radius=%.1f"), InDamage, InInitialSpeed, InRadius));
    }
#endif
}

void AKallari_Skill2A_ShurikenProjectile::InitProjectileArc(
    ATP_Character* InOwnerCharacter,
    UCombatComponent* InCombatComp,
    UObject* InOwningSkill,
    float InDamage,
    const FVector& InLaunchVelocity,
    float InLifeSeconds,
    float InRadius,
    float InGravityScale)
{
    OwnerCharacter = InOwnerCharacter;
    OwningCombatComp = InCombatComp;
    OwningSkill = InOwningSkill;
    Damage = InDamage;

    SetOwner(InOwnerCharacter);
    SetInstigator(InOwnerCharacter);

    Collision->SetSphereRadius(InRadius);
    Collision->IgnoreActorWhenMoving(InOwnerCharacter, true);

    ProjectileMovement->ProjectileGravityScale = InGravityScale;
    ProjectileMovement->InitialSpeed = InLaunchVelocity.Size();
    ProjectileMovement->MaxSpeed = InLaunchVelocity.Size();
    ProjectileMovement->Velocity = InLaunchVelocity;

    SetLifeSpan(InLifeSeconds);

#if !(UE_BUILD_SHIPPING)
    if (UWorld* World = GetWorld())
    {
        DrawDebugSphere(World, GetActorLocation(), InRadius, 16, FColor::Cyan, false, 2.0f, 0, 1.5f);
        DrawDebugLine(World, GetActorLocation(), GetActorLocation() + InLaunchVelocity.GetSafeNormal() * 250.f, FColor::Cyan, false, 2.0f, 0, 2.f);
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, FString::Printf(TEXT("[Projectile Arc Init] Damage=%.1f Speed=%.1f Radius=%.1f Gravity=%.2f"), InDamage, InLaunchVelocity.Size(), InRadius, InGravityScale));
    }
#endif
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

#if !(UE_BUILD_SHIPPING)
        if (UWorld* World = GetWorld())
        {
            DrawDebugSphere(World, HitPoint, Collision ? Collision->GetScaledSphereRadius() : 24.f, 16, FColor::Red, false, 2.0f, 0, 2.0f);
            DrawDebugLine(World, HitPoint, HitPoint + HitNormal * 80.f, FColor::Yellow, false, 2.0f, 0, 2.0f);
        }

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("[Projectile Hit] Target=%s Damage=%.1f"), *GetNameSafe(OtherActor), Damage));
        }
#endif

        OwningCombatComp->ApplyHitToActor(OtherActor, Damage, HitPoint, HitNormal);

        if (UGideon_Skill1B_WaterBomb* WaterBombSkill = Cast<UGideon_Skill1B_WaterBomb>(OwningSkill))
        {
            WaterBombSkill->ExplodeAtLocation(HitPoint);
        }

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

    if (UGideon_Skill1B_WaterBomb* WaterBombSkill = Cast<UGideon_Skill1B_WaterBomb>(OwningSkill))
    {
        FVector ExplosionLocation = GetActorLocation();

        if (ImpactResult.bBlockingHit)
        {
            ExplosionLocation = FVector(ImpactResult.ImpactPoint);
        }

        WaterBombSkill->ExplodeAtLocation(ExplosionLocation);
    }

    ResolveAndDestroy(MarkLocation, MarkNormal);
}

void AKallari_Skill2A_ShurikenProjectile::ResolveAndDestroy(const FVector& MarkLocation, const FVector& MarkNormal)
{
    if (bResolved) return;
    bResolved = true;

    if (UObject* SkillObj = OwningSkill.Get())
    {
        if (UKallari_Skill2A_ShurikenTeleport* Skill2A = Cast<UKallari_Skill2A_ShurikenTeleport>(SkillObj))
        {
            Skill2A->OnProjectileResolved(MarkLocation, MarkNormal);
        }
        else if (UKallari_Skill2B_ShurikenExplosion* Skill2B = Cast<UKallari_Skill2B_ShurikenExplosion>(SkillObj))
        {
            Skill2B->OnProjectileResolved(MarkLocation, MarkNormal);
        }
    }

    Destroy();
}