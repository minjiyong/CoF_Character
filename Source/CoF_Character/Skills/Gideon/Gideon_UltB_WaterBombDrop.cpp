#include "Skills/Gideon/Gideon_UltB_WaterBombDrop.h"

#include "CombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "Skills/Gideon/Gideon_UltB_WaterBombActor.h"
#include "TP_Character.h"

void UGideon_UltB_WaterBombDrop::ResetRuntime()
{
	NextAvailableTime = 0.0;
	bDropActive = false;

	if (IsValid(ActiveBomb))
	{
		ActiveBomb->Destroy();
	}

	ActiveBomb = nullptr;
}

bool UGideon_UltB_WaterBombDrop::IsInCooldown(double Now) const
{
	return Now < NextAvailableTime;
}

void UGideon_UltB_WaterBombDrop::StartCooldown(double Now, float CooldownSec)
{
	NextAvailableTime = Now + CooldownSec;
}

void UGideon_UltB_WaterBombDrop::DropStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector ImpactLocation = FVector::ZeroVector;
	if (!ResolveImpactLocation(ImpactLocation))
	{
		return;
	}

	const FVector SpawnLocation = ImpactLocation + FVector(0.0, 0.0, C->UltB_WaterBombFallHeight);

	TSubclassOf<ACoF_CommonProjectile> BombClass = C->UltB_WaterBombActorClass;
	if (!BombClass)
	{
		BombClass = AGideon_UltB_WaterBombActor::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = C;
	SpawnParams.Instigator = C;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACoF_CommonProjectile* SpawnedProjectile = World->SpawnActor<ACoF_CommonProjectile>(
		BombClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	AGideon_UltB_WaterBombActor* Bomb = Cast<AGideon_UltB_WaterBombActor>(SpawnedProjectile);
	if (!Bomb)
	{
		if (SpawnedProjectile)
		{
			SpawnedProjectile->Destroy();
		}
		return;
	}

	bDropActive = true;
	ActiveBomb = Bomb;

	Bomb->InitVisualBomb(
		this,
		SpawnLocation,
		ImpactLocation,
		C->UltB_WaterBombFallDuration
	);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	DrawDebugSphere(
		World,
		ImpactLocation,
		C->UltB_WaterBombRadius,
		32,
		FColor::Cyan,
		false,
		C->UltB_WaterBombFallDuration
	);

	DrawDebugLine(
		World,
		SpawnLocation,
		ImpactLocation,
		FColor::Blue,
		false,
		C->UltB_WaterBombFallDuration,
		0,
		3.0f
	);
#endif
}

void UGideon_UltB_WaterBombDrop::ExplodeAtLocation(const FVector& ImpactLocation)
{
	ATP_Character* C = GetOwnerChar();
	if (!C || !C->CombatComp)
	{
		bDropActive = false;
		ActiveBomb = nullptr;
		return;
	}

	const float FinalDamage = C->UltB_WaterBombDamage * C->AttackMultiplier;

	C->CombatComp->ConfigureAOELocationHit(
		ImpactLocation,
		FinalDamage,
		C->UltB_WaterBombRadius
	);

	C->CombatComp->BeginHitWindow_OneShot();
	C->CombatComp->EndHitWindow();

	bDropActive = false;
	ActiveBomb = nullptr;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (UWorld* World = GetWorld())
	{
		DrawDebugSphere(
			World,
			ImpactLocation,
			C->UltB_WaterBombRadius,
			32,
			FColor::Red,
			false,
			1.0f
		);
	}
#endif
}

bool UGideon_UltB_WaterBombDrop::ResolveImpactLocation(FVector& OutImpactLocation) const
{
	const ATP_Character* C = GetOwnerChar();
	if (!C)
	{
		return false;
	}

	if (C->HasValidLockOnTarget())
	{
		AActor* Target = C->GetLockOnTarget();
		if (IsValid(Target))
		{
			FVector Origin = FVector::ZeroVector;
			FVector Extent = FVector::ZeroVector;
			Target->GetActorBounds(true, Origin, Extent);

			// 시각적으로는 머리 위에서 떨어지지만, 판정 기준은 대상 발밑 바닥 위치로 잡는다.
			const FVector TargetGroundHint = FVector(
				Origin.X,
				Origin.Y,
				Origin.Z - Extent.Z
			);

			return ProjectToGround(TargetGroundHint, OutImpactLocation);
		}
	}

	const FVector Forward = C->GetActorForwardVector().GetSafeNormal();
	const FVector BaseLocation = C->GetActorLocation() + Forward * C->UltB_WaterBombTargetDistance;

	return ProjectToGround(BaseLocation, OutImpactLocation);
}

bool UGideon_UltB_WaterBombDrop::ProjectToGround(const FVector& SourceLocation, FVector& OutGroundLocation) const
{
	const ATP_Character* C = GetOwnerChar();
	if (!C)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector TraceStart = SourceLocation + FVector(0.0, 0.0, C->UltB_WaterBombGroundTraceUp);
	const FVector TraceEnd = SourceLocation - FVector(0.0, 0.0, C->UltB_WaterBombGroundTraceDown);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GideonUltBProjectToGround), false);
	Params.AddIgnoredActor(C);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	if (bHit)
	{
		OutGroundLocation = Hit.ImpactPoint;
		return true;
	}

	// 바닥 Trace 실패 시에도 스킬이 완전히 무효화되지 않도록 기준 위치를 그대로 사용한다.
	OutGroundLocation = SourceLocation;
	return true;
}

UWorld* UGideon_UltB_WaterBombDrop::GetWorld() const
{
	if (ATP_Character* C = GetOwnerChar())
	{
		return C->GetWorld();
	}

	return nullptr;
}