#include "Skills/Gideon/Gideon_Skill1B_WaterBomb.h"

#include "CombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Projectiles/Kallari_Skill2A_ShurikenProjectile.h"
#include "TP_Character.h"

void UGideon_Skill1B_WaterBomb::ThrowProjectile()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;

	UWorld* World = C->GetWorld();
	if (!World) return;

	TSubclassOf<AKallari_Skill2A_ShurikenProjectile> SpawnClass = C->Skill1B_ProjectileClass;
	if (!SpawnClass)
	{
		SpawnClass = AKallari_Skill2A_ShurikenProjectile::StaticClass();
	}

	FVector SpawnLocation =
		C->GetActorLocation()
		+ C->GetActorForwardVector() * C->Skill1B_ProjectileSpawnForwardOffset
		+ FVector::UpVector * C->Skill1B_ProjectileSpawnZOffset;

	if (USkeletalMeshComponent* MeshComp = C->GetMesh())
	{
		if (C->Skill1B_StartSocket != NAME_None && MeshComp->DoesSocketExist(C->Skill1B_StartSocket))
		{
			SpawnLocation = MeshComp->GetSocketLocation(C->Skill1B_StartSocket);
		}
	}

	// 몸체 전방 기준 + 위쪽 힘을 조금 줘서 포물선 발사
	FVector Forward = C->GetActorForwardVector();
	Forward.Z = 0.f;
	Forward = Forward.GetSafeNormal();

	if (Forward.IsNearlyZero())
	{
		Forward = FVector::ForwardVector;
	}

	const FVector Up = FVector::UpVector;

	// 조절 포인트
	const float ForwardSpeed = C->Skill1B_ProjectileSpeed;
	const float UpwardSpeed = C->Skill1B_ProjectileSpeed * 0.45f;
	const float GravityScale = 1.0f;

	const FVector LaunchVelocity =
		Forward * ForwardSpeed +
		Up * UpwardSpeed;

	FRotator SpawnRotation = LaunchVelocity.Rotation();

	FActorSpawnParameters Params;
	Params.Owner = C;
	Params.Instigator = C;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AKallari_Skill2A_ShurikenProjectile* Projectile =
		World->SpawnActor<AKallari_Skill2A_ShurikenProjectile>(
			SpawnClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

	if (!Projectile) return;

	// 직접 충돌 데미지는 0, 폭발 데미지만 사용
	Projectile->InitProjectileArc(
		C,
		C->CombatComp,
		this,
		0.f,
		LaunchVelocity,
		C->Skill1B_ProjectileLifeSeconds,
		C->Skill1B_ProjectileRadius,
		GravityScale
	);

	ActiveProjectile = Projectile;
	bExplosionConsumed = false;
}

void UGideon_Skill1B_WaterBomb::ExplodeAtLocation(const FVector& ImpactLocation)
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;
	if (bExplosionConsumed) return;

	bExplosionConsumed = true;

	UWorld* World = C->GetWorld();
	if (!World)
	{
		ActiveProjectile = nullptr;
		return;
	}

#if !(UE_BUILD_SHIPPING)
	DrawDebugSphere(
		World,
		ImpactLocation,
		C->Skill1B_Radius,
		24,
		FColor::Blue,
		false,
		1.5f,
		0,
		2.f
	);
#endif

	TArray<FOverlapResult> Overlaps;

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GideonWaterBombAOE), false, C);
	QueryParams.AddIgnoredActor(C);

	const FCollisionShape Sphere = FCollisionShape::MakeSphere(C->Skill1B_Radius);

	if (World->OverlapMultiByObjectType(
		Overlaps,
		ImpactLocation,
		FQuat::Identity,
		ObjParams,
		Sphere,
		QueryParams))
	{
		TSet<AActor*> HitActors;

		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (!HitActor || HitActor == C) continue;
			if (HitActors.Contains(HitActor)) continue;

			HitActors.Add(HitActor);

			FVector HitPoint = HitActor->GetActorLocation();
			FVector HitNormal = (HitActor->GetActorLocation() - ImpactLocation).GetSafeNormal();
			if (HitNormal.IsNearlyZero())
			{
				HitNormal = FVector::UpVector;
			}

			C->CombatComp->ApplyHitToActor(
				HitActor,
				C->Skill1B_Damage * C->AttackMultiplier,
				HitPoint,
				HitNormal
			);
		}
	}

	ActiveProjectile = nullptr;
}