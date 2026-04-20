#include "Skills/Kallari/Kallari_Skill2A_ShurikenTeleport.h"

#include "CombatComponent.h"
#include "GameFramework/PlayerController.h"
#include "Projectiles/Kallari_Skill2A_ShurikenProjectile.h"
#include "TP_Character.h"

void UKallari_Skill2A_ShurikenTeleport::ThrowProjectile()
{
    ATP_Character* C = GetOwnerChar();
    if (!C) return;
    if (!C->GetWorld()) return;
    if (!C->CombatComp) return;

    TSubclassOf<AKallari_Skill2A_ShurikenProjectile> SpawnClass = C->Skill2A_ProjectileClass;
    if (!SpawnClass)
    {
        SpawnClass = AKallari_Skill2A_ShurikenProjectile::StaticClass();
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

    if (APlayerController* PC = Cast<APlayerController>(C->GetController()))
    {
        FVector CamLoc;
        FRotator CamRot;
        PC->GetPlayerViewPoint(CamLoc, CamRot);
        SpawnRotation = CamRot;
    }

    FActorSpawnParameters Params;
    Params.Owner = C;
    Params.Instigator = C;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AKallari_Skill2A_ShurikenProjectile* Projectile =
        C->GetWorld()->SpawnActor<AKallari_Skill2A_ShurikenProjectile>(
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