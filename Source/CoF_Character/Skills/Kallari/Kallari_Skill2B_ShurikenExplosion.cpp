#include "Skills/Kallari/Kallari_Skill2B_ShurikenExplosion.h"

#include "CombatComponent.h"
#include "GameFramework/PlayerController.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "TP_Character.h"

void UKallari_Skill2B_ShurikenExplosion::ThrowProjectile()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->GetWorld()) return;
	if (!C->CombatComp) return;

	TSubclassOf<ACoF_CommonProjectile> SpawnClass = C->Skill2B_ProjectileClass;
	if (!SpawnClass)
	{
		SpawnClass = ACoF_CommonProjectile::StaticClass();
	}

	FVector SpawnLocation = C->GetActorLocation()
		+ C->GetActorForwardVector() * C->Skill2B_ProjectileSpawnForwardOffset
		+ FVector::UpVector * C->Skill2B_ProjectileSpawnZOffset;

	if (C->GetMesh()
		&& C->Skill2B_ProjectileSpawnSocket != NAME_None
		&& C->GetMesh()->DoesSocketExist(C->Skill2B_ProjectileSpawnSocket))
	{
		SpawnLocation = C->GetMesh()->GetSocketLocation(C->Skill2B_ProjectileSpawnSocket);
	}

	FRotator SpawnRotation = C->GetActorRotation();

	// 락온 대상이 있으면 원래대로 대상에게 발사
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
			Params
		);

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