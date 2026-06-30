#include "Skills/Gideon/Gideon_Skill1B_WaterBomb.h"

#include "CombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "TP_Character.h"

namespace
{
	void SpawnGideonAOEWaterExplosionFX(ATP_Character* C, const FVector& ImpactLocation, float Radius)
	{
		if (!C)
		{
			return;
		}

		if (!C->GideonAOEWaterExplosionFXClass)
		{
			return;
		}

		UWorld* World = C->GetWorld();
		if (!World)
		{
			return;
		}

		FActorSpawnParameters Params;
		Params.Owner = C;
		Params.Instigator = C;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedFX = World->SpawnActor<AActor>(
			C->GideonAOEWaterExplosionFXClass,
			ImpactLocation + FVector(0.f, 0.f, 5.f),
			FRotator::ZeroRotator,
			Params
		);

		if (!SpawnedFX)
		{
			return;
		}

		const float BaseRadius = FMath::Max(C->GideonAOEWaterExplosionFXBaseRadius, 1.f);
		const float FXScale = FMath::Max(Radius / BaseRadius, 0.1f);

		SpawnedFX->SetActorScale3D(FVector(FXScale));
	}
}

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
	const float GravityScale = FMath::Max(C->Skill1B_ProjectileGravityScale, 0.01f);
	const float ArcPeakHeight = FMath::Max(C->Skill1B_ProjectileLockOnArcPeakHeight, 50.f);

	FVector LaunchVelocity = FVector::ZeroVector;

	// 락온 대상이 있으면: 손 소켓 -> 락온 대상 하체 근처를 정확히 통과하는 포물선
	if (C->HasValidLockOnTarget())
	{
		if (AActor* LockTarget = C->GetLockOnTarget())
		{
			FVector TargetOrigin, TargetExtent;
			LockTarget->GetActorBounds(true, TargetOrigin, TargetExtent);

			// 중심이 아니라 하체 쪽으로 조준
			FVector TargetPoint = TargetOrigin;
			TargetPoint.Z -= TargetExtent.Z * 0.5f;

			FVector HorizontalDelta = TargetPoint - SpawnLocation;
			HorizontalDelta.Z = 0.f;

			FVector HorizontalDir = HorizontalDelta.GetSafeNormal2D();
			if (HorizontalDir.IsNearlyZero())
			{
				HorizontalDir = C->GetActorForwardVector().GetSafeNormal2D();
			}

			const float HorizontalDist = FMath::Max(HorizontalDelta.Size(), 1.f);
			const float Gravity = FMath::Abs(World->GetGravityZ()) * GravityScale;

			// 정점을 목표점/시작점보다 ArcPeakHeight 만큼 높게
			const float ApexZ = FMath::Max(SpawnLocation.Z, TargetPoint.Z) + ArcPeakHeight;

			const float HeightToApex = FMath::Max(ApexZ - SpawnLocation.Z, 1.f);
			const float HeightFromApex = FMath::Max(ApexZ - TargetPoint.Z, 1.f);

			const float TimeUp = FMath::Sqrt((2.f * HeightToApex) / Gravity);
			const float TimeDown = FMath::Sqrt((2.f * HeightFromApex) / Gravity);
			const float TotalTime = FMath::Max(TimeUp + TimeDown, 0.05f);

			const FVector HorizontalVelocity = HorizontalDir * (HorizontalDist / TotalTime);
			const float VerticalVelocity = Gravity * TimeUp;

			LaunchVelocity = HorizontalVelocity + FVector::UpVector * VerticalVelocity;
		}
	}
	else
	{
		// 비락온: 몸체 전방 + 위쪽 힘
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

	// 이펙트 생성
	SpawnGideonAOEWaterExplosionFX(C, ImpactLocation, C->Skill1B_Radius);

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