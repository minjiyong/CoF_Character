#include "Skills/Kallari/Kallari_Skill2B_ShurikenExplosion.h"

#include "CombatComponent.h"
#include "GameFramework/PlayerController.h"
#include "Projectiles/Kallari_Skill2A_ShurikenProjectile.h"
#include "TP_Character.h"

void UKallari_Skill2B_ShurikenExplosion::ThrowProjectile()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;
    if (!C->GetWorld()) return;
    if (!C->CombatComp) return;

    TSubclassOf<AKallari_Skill2A_ShurikenProjectile> SpawnClass = C->Skill2B_ProjectileClass;
    if (!SpawnClass)
    {
        SpawnClass = AKallari_Skill2A_ShurikenProjectile::StaticClass();
    }

    FVector SpawnLocation =
        C->GetActorLocation()
        + C->GetActorForwardVector() * C->Skill2B_ProjectileSpawnForwardOffset
        + FVector::UpVector * C->Skill2B_ProjectileSpawnZOffset;

    if (C->GetMesh()
        && C->Skill2B_ProjectileSpawnSocket != NAME_None
        && C->GetMesh()->DoesSocketExist(C->Skill2B_ProjectileSpawnSocket))
    {
        SpawnLocation = C->GetMesh()->GetSocketLocation(C->Skill2B_ProjectileSpawnSocket);
    }

    FRotator SpawnRotation = C->GetActorRotation();

    if (C->HasValidLockOnTarget())
    {
        AActor* LockTarget = C->GetLockOnTarget();

        FVector TargetOrigin, TargetExtent;
        LockTarget->GetActorBounds(true, TargetOrigin, TargetExtent);

        const FVector ShootDir = (TargetOrigin - SpawnLocation).GetSafeNormal();
        if (!ShootDir.IsNearlyZero())
        {
            SpawnRotation = ShootDir.Rotation();
        }
    }
    else if (APlayerController* PC = Cast<APlayerController>(C->GetController()))
    {
        FVector CamLoc;
        FRotator CamRot;
        PC->GetPlayerViewPoint(CamLoc, CamRot);

        const FVector TraceStart = CamLoc;
        const FVector TraceEnd = TraceStart + CamRot.Vector() * 10000.f;

        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(KallariSkill2B_AimTrace), false, C);
        Params.AddIgnoredActor(C);

        FVector TargetPoint = TraceEnd;
        if (C->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            TargetPoint = Hit.ImpactPoint;
        }

        const FVector ShootDir = (TargetPoint - SpawnLocation).GetSafeNormal();
        if (!ShootDir.IsNearlyZero())
        {
            SpawnRotation = ShootDir.Rotation();
        }
    }

    FActorSpawnParameters Params;
    Params.Owner = C;
    Params.Instigator = C;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AKallari_Skill2A_ShurikenProjectile* Projectile =
        C->GetWorld()->SpawnActor<AKallari_Skill2A_ShurikenProjectile>(
            SpawnClass, SpawnLocation, SpawnRotation, Params);

    if (!Projectile) return;

    Projectile->InitProjectile(
        C,
        C->CombatComp,
        this,
        C->Skill2B_ExplosionDamage * C->AttackMultiplier,
        C->Skill2B_ProjectileSpeed,
        C->Skill2B_ProjectileLifeSeconds,
        C->Skill2B_ProjectileRadius
    );

    ActiveProjectile = Projectile;
    bHasExplosionMark = false;
}

void UKallari_Skill2B_ShurikenExplosion::OnProjectileResolved(const FVector& InMarkLocation, const FVector& InMarkNormal)
{
    bHasExplosionMark = true;
    ExplosionMarkLocation = InMarkLocation;
    ExplosionMarkNormal = InMarkNormal;
    ActiveProjectile.Reset();
}

bool UKallari_Skill2B_ShurikenExplosion::PlayExplosionMontage()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return false;
    if (!bHasExplosionMark) return false;
    if (!C->Skill2B_ExplosionMontage) return false;

    C->PlayAnimMontage(C->Skill2B_ExplosionMontage);
    return true;
}

void UKallari_Skill2B_ShurikenExplosion::ExplodeAtMark()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;
    if (!C->CombatComp) return;
    if (!bHasExplosionMark) return;

    C->CombatComp->ConfigureAOELocationHit(
        ExplosionMarkLocation,
        C->Skill2B_ExplosionDamage * C->AttackMultiplier,
        C->Skill2B_ExplosionRadius
    );

    C->CombatComp->BeginHitWindow_OneShot();

    bHasExplosionMark = false;
}