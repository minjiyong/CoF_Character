#include "Skills/Gideon/Gideon_Skill2A_DebuffBall.h"

#include "CombatComponent.h"
#include "Interfaces/DebuffBallTargetInterface.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "TP_Character.h"

void UGideon_Skill2A_DebuffBall::ThrowProjectile()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;

	UWorld* World = C->GetWorld();
	if (!World) return;
	if (!C->Skill2A_ProjectileClass) return;

	FVector SpawnLocation =
		C->GetActorLocation()
		+ C->GetActorForwardVector() * C->Skill2A_ProjectileSpawnForwardOffset
		+ FVector::UpVector * C->Skill2A_ProjectileSpawnZOffset;

	FRotator SpawnRotation = C->GetActorRotation();

	if (USkeletalMeshComponent* MeshComp = C->GetMesh())
	{
		if (C->Skill2A_ProjectileSpawnSocket != NAME_None &&
			MeshComp->DoesSocketExist(C->Skill2A_ProjectileSpawnSocket))
		{
			SpawnLocation = MeshComp->GetSocketLocation(C->Skill2A_ProjectileSpawnSocket);
		}
	}

	// 평타 throwprojectile 기본형 그대로
	if (C->HasValidLockOnTarget())
	{
		if (AActor* LockTarget = C->GetLockOnTarget())
		{
			FVector TargetOrigin, TargetExtent;
			LockTarget->GetActorBounds(true, TargetOrigin, TargetExtent);

			const FVector ShootDir = (TargetOrigin - SpawnLocation).GetSafeNormal();
			if (!ShootDir.IsNearlyZero())
			{
				SpawnRotation = ShootDir.Rotation();
			}
		}
	}
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
		World->SpawnActor<ACoF_CommonProjectile>(
			C->Skill2A_ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

	if (!Projectile) return;

	// 직접 데미지는 0, 적중 시 Debuff만 부여
	Projectile->InitProjectile(
		C,
		C->CombatComp,
		this,
		0.f,
		C->Skill2A_ProjectileSpeed,
		C->Skill2A_ProjectileLifeSeconds,
		C->Skill2A_ProjectileRadius
	);
}

void UGideon_Skill2A_DebuffBall::ApplyDebuffToActor(AActor* Target)
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!Target) return;

	if (Target->GetClass()->ImplementsInterface(UDebuffBallTargetInterface::StaticClass()))
	{
		IDebuffBallTargetInterface::Execute_ApplyDebuffBall(
			Target,
			C->Skill2A_DebuffDuration,
			C->Skill2A_DebuffIncomingDamageMultiplier
		);
	}
}