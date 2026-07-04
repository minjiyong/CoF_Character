#include "Skills/Kallari/Kallari_Skill2B_ShurikenExplosion.h"

#include "CombatComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "TP_Character.h"

namespace
{
	// Kallari Skill2B의 실제 폭발 위치에 FX Actor를 생성한다.
	void SpawnKallariSkill2BExplosionFX(
		ATP_Character* C,
		const FVector& ImpactLocation
	)
	{
		if (!C)
		{
			return;
		}

		if (!C->KallariSkill2B_ExplosionFXClass)
		{
			return;
		}

		UWorld* World = C->GetWorld();

		if (!World)
		{
			return;
		}

		const FVector SpawnLocation =
			ImpactLocation
			+ FVector::UpVector
			* C->KallariSkill2B_ExplosionFXZOffset;

		FActorSpawnParameters Params;
		Params.Owner = C;
		Params.Instigator = C;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedFX = World->SpawnActor<AActor>(
			C->KallariSkill2B_ExplosionFXClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			Params
		);

		if (!SpawnedFX)
		{
			return;
		}

		// 연막이나 폭발 이펙트가 종료된 뒤 빈 Actor가 남지 않게 한다.
		if (C->KallariSkill2B_ExplosionFXLifeSeconds > 0.f)
		{
			SpawnedFX->SetLifeSpan(
				C->KallariSkill2B_ExplosionFXLifeSeconds
			);
		}
	}
}

void UKallari_Skill2B_ShurikenExplosion::ThrowProjectile()
{
	ATP_Character* C = GetOwnerChar();

	if (!C) return;
	if (!C->GetWorld()) return;
	if (!C->CombatComp) return;

	TSubclassOf<ACoF_CommonProjectile> SpawnClass =
		C->Skill2B_ProjectileClass;

	if (!SpawnClass)
	{
		SpawnClass = ACoF_CommonProjectile::StaticClass();
	}

	FVector SpawnLocation =
		C->GetActorLocation()
		+ C->GetActorForwardVector()
		* C->Skill2B_ProjectileSpawnForwardOffset
		+ FVector::UpVector
		* C->Skill2B_ProjectileSpawnZOffset;

	if (
		C->GetMesh()
		&& C->Skill2B_ProjectileSpawnSocket != NAME_None
		&& C->GetMesh()->DoesSocketExist(
			C->Skill2B_ProjectileSpawnSocket
		)
		)
	{
		SpawnLocation =
			C->GetMesh()->GetSocketLocation(
				C->Skill2B_ProjectileSpawnSocket
			);
	}

	FRotator SpawnRotation = C->GetActorRotation();

	// 락온 대상이 있으면 원래대로 대상에게 발사
	if (C->HasValidLockOnTarget())
	{
		AActor* LockTarget = C->GetLockOnTarget();

		FVector TargetOrigin;
		FVector TargetExtent;

		LockTarget->GetActorBounds(
			true,
			TargetOrigin,
			TargetExtent
		);

		const FVector ShootDir =
			(TargetOrigin - SpawnLocation).GetSafeNormal();

		if (!ShootDir.IsNearlyZero())
		{
			SpawnRotation = ShootDir.Rotation();
		}
	}
	// 락온이 없으면 지면 수평 발사
	else
	{
		FVector HorizontalDir =
			C->GetActorForwardVector();

		HorizontalDir.Z = 0.f;
		HorizontalDir =
			HorizontalDir.GetSafeNormal();

		if (!HorizontalDir.IsNearlyZero())
		{
			SpawnRotation =
				HorizontalDir.Rotation();
		}
	}

	FActorSpawnParameters Params;
	Params.Owner = C;
	Params.Instigator = C;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACoF_CommonProjectile* Projectile =
		C->GetWorld()->SpawnActor<ACoF_CommonProjectile>(
			SpawnClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

	if (!Projectile)
	{
		return;
	}

	Projectile->InitProjectile(
		C,
		C->CombatComp,
		this,
		C->Skill2B_ExplosionDamage
		* C->AttackMultiplier,
		C->Skill2B_ProjectileSpeed,
		C->Skill2B_ProjectileLifeSeconds,
		C->Skill2B_ProjectileRadius
	);

	ActiveProjectile = Projectile;
	bHasExplosionMark = false;
}

void UKallari_Skill2B_ShurikenExplosion::OnProjectileResolved(
	const FVector& InMarkLocation,
	const FVector& InMarkNormal
)
{
	bHasExplosionMark = true;

	ExplosionMarkLocation = InMarkLocation;
	ExplosionMarkNormal = InMarkNormal;

	ActiveProjectile.Reset();
}

bool UKallari_Skill2B_ShurikenExplosion::PlayExplosionMontage()
{
	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return false;
	}

	if (!bHasExplosionMark)
	{
		return false;
	}

	if (!C->Skill2B_ExplosionMontage)
	{
		return false;
	}

	C->PlayAnimMontage(
		C->Skill2B_ExplosionMontage
	);

	return true;
}

void UKallari_Skill2B_ShurikenExplosion::ExplodeAtMark()
{
	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return;
	}

	if (!C->CombatComp)
	{
		return;
	}

	if (!bHasExplosionMark)
	{
		return;
	}

	// 위치는 아래에서 초기화하기 전에 별도로 보관한다.
	const FVector ImpactLocation =
		ExplosionMarkLocation;

	// 실제 폭발 피해 판정과 동일한 위치에 FX를 생성한다.
	SpawnKallariSkill2BExplosionFX(
		C,
		ImpactLocation
	);

	C->CombatComp->ConfigureAOELocationHit(
		ImpactLocation,
		C->Skill2B_ExplosionDamage
		* C->AttackMultiplier,
		C->Skill2B_ExplosionRadius
	);

	C->CombatComp->BeginHitWindow_OneShot();

	bHasExplosionMark = false;
}