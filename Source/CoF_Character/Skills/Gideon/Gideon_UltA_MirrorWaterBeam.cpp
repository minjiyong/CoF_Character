#include "Skills/Gideon/Gideon_UltA_MirrorWaterBeam.h"

#include "CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "TP_Character.h"

UWorld* UGideon_UltA_MirrorWaterBeam::GetWorld() const
{
	if (ATP_Character* C = GetOwnerChar())
	{
		return C->GetWorld();
	}

	return nullptr;
}

void UGideon_UltA_MirrorWaterBeam::ResetRuntime()
{
	NextAvailableTime = 0.0;
	bBeamActive = false;

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(BeamTickTimerHandle);
		W->GetTimerManager().ClearTimer(BeamEndTimerHandle);
	}

	ClearWaterBeamFX();
}

void UGideon_UltA_MirrorWaterBeam::BeamStart()
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

	if (bBeamActive)
	{
		return;
	}

	UWorld* W = GetWorld();

	if (!W)
	{
		return;
	}

	bBeamActive = true;

	C->SetMoveInputEnabled(false);
	C->SetAttackInputEnabled(false);
	C->SetGuardInputEnabled(false);
	C->SetSkillInputEnabled(false);
	C->SetJumpInputEnabled(false);

	SpawnWaterBeamFX();

	BeamTick();

	W->GetTimerManager().SetTimer(
		BeamTickTimerHandle,
		this,
		&UGideon_UltA_MirrorWaterBeam::BeamTick,
		C->UltA_BeamTickInterval,
		true
	);

	W->GetTimerManager().SetTimer(
		BeamEndTimerHandle,
		this,
		&UGideon_UltA_MirrorWaterBeam::BeamEnd,
		C->UltA_BeamDuration,
		false
	);
}

void UGideon_UltA_MirrorWaterBeam::BeamTick()
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

	if (!bBeamActive)
	{
		return;
	}

	C->CombatComp->ConfigureBeamSweepHit(
		C->UltA_BeamDamagePerTick * C->AttackMultiplier,
		C->UltA_BeamRange,
		C->UltA_BeamRadius,
		C->UltA_BeamStartSocket
	);

	C->CombatComp->BeginHitWindow_OneShot();
}

void UGideon_UltA_MirrorWaterBeam::BeamEnd()
{
	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return;
	}

	UWorld* W = GetWorld();

	if (W)
	{
		W->GetTimerManager().ClearTimer(BeamTickTimerHandle);
		W->GetTimerManager().ClearTimer(BeamEndTimerHandle);
	}

	ClearWaterBeamFX();

	bBeamActive = false;

	C->SetEveryInputEnabled(true);
}

void UGideon_UltA_MirrorWaterBeam::SpawnWaterBeamFX()
{
	ClearWaterBeamFX();

	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return;
	}

	if (!C->GideonUltA_WaterBeamFXClass)
	{
		return;
	}

	UWorld* W = GetWorld();

	if (!W)
	{
		return;
	}

	const FVector StartLocation = ResolveBeamFXStartLocation();
	const FVector EndLocation = ResolveBeamFXEndLocation(StartLocation);

	FVector Direction = EndLocation - StartLocation;
	const float BeamLength = Direction.Size();

	if (BeamLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	Direction /= BeamLength;

	const FRotator SpawnRotation = Direction.Rotation();

	FActorSpawnParameters Params;
	Params.Owner = C;
	Params.Instigator = C;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveWaterBeamFX = W->SpawnActor<AActor>(
		C->GideonUltA_WaterBeamFXClass,
		StartLocation,
		SpawnRotation,
		Params
	);

	if (!ActiveWaterBeamFX)
	{
		return;
	}

	const float BaseLength = FMath::Max(C->GideonUltA_WaterBeamFXBaseLength, 1.f);
	const float LengthScale = FMath::Max(BeamLength / BaseLength, 0.01f);
	const float ThicknessScale = FMath::Max(C->GideonUltA_WaterBeamFXThickness, 0.01f);

	// BP_UltA_WaterBeamFX는 Actor의 +X 방향으로 빔이 나가도록 제작한다.
	// X 스케일은 길이, Y/Z 스케일은 두께 보정으로 사용한다.
	ActiveWaterBeamFX->SetActorScale3D(
		FVector(
			LengthScale,
			ThicknessScale,
			ThicknessScale
		)
	);
}

void UGideon_UltA_MirrorWaterBeam::ClearWaterBeamFX()
{
	if (IsValid(ActiveWaterBeamFX))
	{
		ActiveWaterBeamFX->Destroy();
	}

	ActiveWaterBeamFX = nullptr;
}

FVector UGideon_UltA_MirrorWaterBeam::ResolveBeamFXStartLocation() const
{
	const ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return FVector::ZeroVector;
	}

	const USkeletalMeshComponent* MeshComp = C->GetMesh();

	if (MeshComp && C->UltA_BeamStartSocket != NAME_None && MeshComp->DoesSocketExist(C->UltA_BeamStartSocket))
	{
		return MeshComp->GetSocketLocation(C->UltA_BeamStartSocket);
	}

	return C->GetActorLocation() + C->GetActorForwardVector() * 50.f + FVector(0.f, 0.f, 100.f);
}

FVector UGideon_UltA_MirrorWaterBeam::ResolveBeamFXEndLocation(const FVector& StartLocation) const
{
	const ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return StartLocation;
	}

	FVector Direction = C->GetActorForwardVector();

	if (C->HasValidLockOnTarget())
	{
		AActor* Target = C->GetLockOnTarget();

		if (IsValid(Target))
		{
			FVector TargetOrigin = Target->GetActorLocation();
			FVector TargetExtent = FVector::ZeroVector;

			Target->GetActorBounds(true, TargetOrigin, TargetExtent);

			const FVector ToTarget = TargetOrigin - StartLocation;

			if (!ToTarget.IsNearlyZero())
			{
				Direction = ToTarget.GetSafeNormal();
			}
		}
	}

	if (Direction.IsNearlyZero())
	{
		Direction = C->GetActorForwardVector();
	}

	return StartLocation + Direction.GetSafeNormal() * C->UltA_BeamRange;
}