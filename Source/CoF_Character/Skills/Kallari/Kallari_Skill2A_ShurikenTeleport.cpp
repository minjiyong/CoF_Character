#include "Skills/Kallari/Kallari_Skill2A_ShurikenTeleport.h"

#include "CombatComponent.h"
#include "GameFramework/PlayerController.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "TP_Character.h"

void UKallari_Skill2A_ShurikenTeleport::ThrowProjectile()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;
    if (!C->GetWorld()) return;
    if (!C->CombatComp) return;

    TSubclassOf<ACoF_CommonProjectile> SpawnClass = C->Skill2A_ProjectileClass;
    if (!SpawnClass)
    {
        SpawnClass = ACoF_CommonProjectile::StaticClass();
    }

    FVector SpawnLocation =
        C->GetActorLocation()
        + C->GetActorForwardVector() * C->Skill2A_ProjectileSpawnForwardOffset
        + FVector::UpVector * C->Skill2A_ProjectileSpawnZOffset;

    if (C->GetMesh()
        && C->Skill2A_ProjectileSpawnSocket != NAME_None
        && C->GetMesh()->DoesSocketExist(C->Skill2A_ProjectileSpawnSocket))
    {
        SpawnLocation = C->GetMesh()->GetSocketLocation(C->Skill2A_ProjectileSpawnSocket);
    }

    FRotator SpawnRotation = C->GetActorRotation();

    // 락온 대상이 있으면 대상에게 발사
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

    // 락온이 없으면 지면 수평 발사
    else
    {
        FVector HorizontalDir = C->GetActorForwardVector();
        HorizontalDir.Z = 0.f;
        HorizontalDir = HorizontalDir.GetSafeNormal();

        if (!HorizontalDir.IsNearlyZero())
        {
            SpawnRotation = HorizontalDir.Rotation();
        }
    }

    FActorSpawnParameters Params;
    Params.Owner = C;
    Params.Instigator = C;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ACoF_CommonProjectile* Projectile =
        C->GetWorld()->SpawnActor<ACoF_CommonProjectile>(
            SpawnClass,
            SpawnLocation,
            SpawnRotation,
            Params);

    if (!Projectile) return;

    Projectile->InitProjectile(
        C,
        C->CombatComp,
        this,
        C->Skill2A_Damage * C->AttackMultiplier,
        C->Skill2A_ProjectileSpeed,
        C->Skill2A_ProjectileLifeSeconds,
        C->Skill2A_ProjectileRadius);

    ActiveProjectile = Projectile;
    bHasTeleportMark = false;
}

void UKallari_Skill2A_ShurikenTeleport::OnProjectileResolved(const FVector& InMarkLocation, const FVector& InMarkNormal)
{
    bHasTeleportMark = true;
    TeleportMarkLocation = InMarkLocation;
    TeleportMarkNormal = InMarkNormal;
    ActiveProjectile.Reset();
}

bool UKallari_Skill2A_ShurikenTeleport::TeleportToMarkAndAttack()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return false;
    if (!bHasTeleportMark) return false;

    FVector Dest = TeleportMarkLocation;

    const FVector SafeNormal = TeleportMarkNormal.GetSafeNormal();
    if (!SafeNormal.IsNearlyZero())
    {
        Dest += SafeNormal * C->Skill2A_TeleportOffsetFromMark;
    }

    C->SetActorLocation(Dest, false, nullptr, ETeleportType::TeleportPhysics);

    if (C->HasValidLockOnTarget())
    {
        FVector ToTarget = C->GetLockOnTarget()->GetActorLocation() - C->GetActorLocation();
        ToTarget.Z = 0.f;

        if (!ToTarget.IsNearlyZero())
        {
            C->SetActorRotation(ToTarget.Rotation());
        }
    }

    bHasTeleportMark = false;
    ActiveProjectile.Reset();

    if (C->Skill2A_TeleportAttackMontage)
    {
        C->PlayAnimMontage(C->Skill2A_TeleportAttackMontage);
        return true;
    }

    return false;
}

void UKallari_Skill2A_ShurikenTeleport::HitStart()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;
    if (!C->CombatComp) return;

    C->CombatComp->ConfigureAOEHit(
        C->Skill2A_Damage * C->AttackMultiplier,
        C->Skill2A_TeleportAttackRadius
    );

    C->CombatComp->BeginHitWindow_OneShot();
}