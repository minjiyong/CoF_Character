#include "Skills/Gideon/Gideon_Skill1B_WaterBomb.h"

#include "CombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "TP_Character.h"

void UGideon_Skill1B_WaterBomb::ThrowProjectile()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;

	UWorld* World = C->GetWorld();
	if (!World) return;

	TSubclassOf<ACoF_CommonProjectile> SpawnClass = C->Skill1B_ProjectileClass;
	if (!SpawnClass)
	{
		SpawnClass = ACoF_CommonProjectile::StaticClass();
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

	const float ForwardSpeed = FMath::Max(C->Skill1B_ProjectileForwardSpeed, 1.f);
	const float UpwardSpeed = C->Skill1B_ProjectileUpwardSpeed;
	const float GravityScale = C->Skill1B_ProjectileGravityScale;
	const float LockOnExtraUpward = C->Skill1B_ProjectileLockOnExtraUpwardSpeed;

	FVector LaunchVelocity = FVector::ZeroVector;

	// 락온 대상이 있으면: 손 소켓 -> 락온 대상 중심으로 포물선 연결
	if (C->HasValidLockOnTarget())
	{
		if (AActor* LockTarget = C->GetLockOnTarget())
		{
			FVector TargetOrigin, TargetExtent;
			LockTarget->GetActorBounds(true, TargetOrigin, TargetExtent);

			FVector Delta = TargetOrigin - SpawnLocation;
			FVector Horizontal = Delta;
			Horizontal.Z = 0.f;

			FVector HorizontalDir = Horizontal.GetSafeNormal();
			if (HorizontalDir.IsNearlyZero())
			{
				HorizontalDir = C->GetActorForwardVector().GetSafeNormal2D();
			}

			const float HorizontalDist = FMath::Max(Horizontal.Size(), 1.f);
			const float Gravity = FMath::Abs(World->GetGravityZ()) * GravityScale;

			// 전방 속도로 비행 시간 계산
			const float FlightTime = HorizontalDist / ForwardSpeed;

			// 목표점에 닿게 하는 수직 속도 + 상향 보정
			float VerticalSpeed = 0.f;
			if (FlightTime > KINDA_SMALL_NUMBER)
			{
				VerticalSpeed =
					(Delta.Z + 0.5f * Gravity * FlightTime * FlightTime) / FlightTime
					+ LockOnExtraUpward;
			}
			else
			{
				VerticalSpeed = UpwardSpeed + LockOnExtraUpward;
			}

			LaunchVelocity = HorizontalDir * ForwardSpeed + FVector::UpVector * VerticalSpeed;
		}
	}
	else
	{
		// 락온이 없으면: 몸체 전방 + 위쪽 힘
		FVector Forward = C->GetActorForwardVector();
		Forward.Z = 0.f;
		Forward = Forward.GetSafeNormal();

		if (Forward.IsNearlyZero())
		{
			Forward = FVector::ForwardVector;
		}

		LaunchVelocity = Forward * ForwardSpeed + FVector::UpVector * UpwardSpeed;
	}

	FRotator SpawnRotation = LaunchVelocity.Rotation();

	FActorSpawnParameters Params;
	Params.Owner = C;
	Params.Instigator = C;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACoF_CommonProjectile* Projectile =
		World->SpawnActor<ACoF_CommonProjectile>(
			SpawnClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

	if (!Projectile) return;

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