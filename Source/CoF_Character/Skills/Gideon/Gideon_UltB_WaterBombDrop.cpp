#include "Skills/Gideon/Gideon_UltB_WaterBombDrop.h"

#include "CombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "TP_Character.h"

void UGideon_UltB_WaterBombDrop::ResetRuntime()
{
	CooldownEndTime = 0.0;
	ActiveBomb = nullptr;
}

bool UGideon_UltB_WaterBombDrop::IsInCooldown(double Now) const
{
	return Now < CooldownEndTime;
}

void UGideon_UltB_WaterBombDrop::StartCooldown(double Now, float Cooldown)
{
	CooldownEndTime = Now + Cooldown;
}

void UGideon_UltB_WaterBombDrop::DropStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C)
	{
		return;
	}

	UWorld* World = GetWorldFromOwner();
	if (!World)
	{
		return;
	}

	const FVector ImpactLocation = ResolveImpactLocation();
	const FVector SpawnLocation = ImpactLocation + FVector(0.f, 0.f, C->UltB_WaterBombFallHeight);

	TSubclassOf<ACoF_CommonProjectile> BombClass = C->UltB_WaterBombActorClass;
	if (!BombClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Red,
				TEXT("[Gideon UltB] UltB_WaterBombActorClass is null. Check CharacterData cache and DA_Char02_Gideon.")
			);
		}

		// 물폭탄 BP가 연결되지 않아도 스킬 판정이 완전히 증발하지 않도록 즉시 AOE 폭발 처리
		ExplodeAtLocation(ImpactLocation);
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = C;
	SpawnParams.Instigator = C;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACoF_CommonProjectile* Bomb = World->SpawnActor<ACoF_CommonProjectile>(
		BombClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!Bomb)
	{
		ExplodeAtLocation(ImpactLocation);
		return;
	}

	ActiveBomb = Bomb;

	const float FallDuration = FMath::Max(C->UltB_WaterBombFallDuration, 0.01f);
	const float FallSpeed = C->UltB_WaterBombFallHeight / FallDuration;

	const FVector LaunchVelocity = FVector(0.f, 0.f, -FallSpeed);

	Bomb->InitProjectileArc(
		C,
		C->CombatComp,
		this,
		0.f,
		LaunchVelocity,
		FallDuration + 1.0f,
		C->UltB_WaterBombRadius,
		0.f
	);

	DrawDebugSphere(World, ImpactLocation, C->UltB_WaterBombRadius, 32, FColor::Cyan, false, 2.0f);
	DrawDebugLine(World, SpawnLocation, ImpactLocation, FColor::Cyan, false, 2.0f, 0, 3.0f);
}

void UGideon_UltB_WaterBombDrop::ExplodeAtLocation(const FVector& ImpactLocation)
{
	ATP_Character* C = GetOwnerChar();
	if (!C || !C->CombatComp)
	{
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

	if (UWorld* World = GetWorldFromOwner())
	{
		DrawDebugSphere(World, ImpactLocation, C->UltB_WaterBombRadius, 48, FColor::Red, false, 2.0f);
	}
}

UWorld* UGideon_UltB_WaterBombDrop::GetWorldFromOwner() const
{
	const ATP_Character* C = GetOwnerChar();
	return C ? C->GetWorld() : nullptr;
}

FVector UGideon_UltB_WaterBombDrop::ResolveImpactLocation() const
{
	const ATP_Character* C = GetOwnerChar();
	if (!C)
	{
		return FVector::ZeroVector;
	}

	FVector SourceLocation = C->GetActorLocation();

	if (C->HasValidLockOnTarget())
	{
		SourceLocation = C->LockOnTarget->GetActorLocation();
	}
	else
	{
		SourceLocation = C->GetActorLocation() + C->GetActorForwardVector() * C->UltB_WaterBombTargetDistance;
	}

	FVector GroundLocation;
	if (ProjectToGround(SourceLocation, GroundLocation))
	{
		return GroundLocation;
	}

	return SourceLocation;
}

bool UGideon_UltB_WaterBombDrop::ProjectToGround(const FVector& SourceLocation, FVector& OutGroundLocation) const
{
	const ATP_Character* C = GetOwnerChar();
	UWorld* World = GetWorldFromOwner();

	if (!C || !World)
	{
		return false;
	}

	const FVector TraceStart = SourceLocation + FVector(0.f, 0.f, C->UltB_WaterBombGroundTraceUp);
	const FVector TraceEnd = SourceLocation - FVector(0.f, 0.f, C->UltB_WaterBombGroundTraceDown);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GideonUltBProjectToGround), false, C);

	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	if (!bHit)
	{
		return false;
	}

	OutGroundLocation = Hit.ImpactPoint;
	return true;
}