#include "Skills/Gideon/Gideon_Skill1A_WaterCannon.h"

#include "CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TP_Character.h"

void UGideon_Skill1A_WaterCannon::ResetRuntime()
{
	NextAvailableTime = 0.0;
	ClearWaterBeamFX();
}

bool UGideon_Skill1A_WaterCannon::IsInCooldown(double Now) const
{
	return Now < NextAvailableTime;
}

void UGideon_Skill1A_WaterCannon::StartCooldown(double Now, float CooldownSec)
{
	NextAvailableTime = Now + CooldownSec;
}

void UGideon_Skill1A_WaterCannon::HitStart()
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

	C->CombatComp->ConfigureTraceHit(
		C->Skill1A_Damage * C->AttackMultiplier,
		C->Skill1A_Range,
		C->Skill1A_StartSocket
	);

	C->CombatComp->BeginHitWindow_OneShot();

	SpawnWaterBeamFX();
}

void UGideon_Skill1A_WaterCannon::HitEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (C && C->CombatComp)
	{
		C->CombatComp->EndHitWindow();
	}

	ClearWaterBeamFX();
}

void UGideon_Skill1A_WaterCannon::SpawnWaterBeamFX()
{
	ClearWaterBeamFX();

	ATP_Character* C = GetOwnerChar();
	if (!C)
	{
		return;
	}

	if (!C->GideonSkill1A_WaterBeamFXClass)
	{
		return;
	}

	UWorld* World = C->GetWorld();
	if (!World)
	{
		return;
	}

	const FVector StartLocation = ResolveFXStartLocation();
	const FVector EndLocation = ResolveFXEndLocation(StartLocation);

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

	ActiveWaterBeamFX = World->SpawnActor<AActor>(
		C->GideonSkill1A_WaterBeamFXClass,
		StartLocation,
		SpawnRotation,
		Params
	);

	if (!ActiveWaterBeamFX)
	{
		return;
	}

	const float SafeBaseLength = FMath::Max(C->GideonSkill1A_WaterBeamFXBaseLength, 1.f);
	const float LengthScale = BeamLength / SafeBaseLength;

	// BP_Gideon_Skill1A_WaterBeamFX가 로컬 X축 방향으로 길게 만들어져 있다는 전제.
	// 방향이 맞지 않으면 BP 안의 Spline/Niagara 방향을 X축 기준으로 맞춘다.
	ActiveWaterBeamFX->SetActorScale3D(
		FVector(
			LengthScale,
			C->GideonSkill1A_WaterBeamFXThickness,
			C->GideonSkill1A_WaterBeamFXThickness
		)
	);
}

void UGideon_Skill1A_WaterCannon::ClearWaterBeamFX()
{
	if (IsValid(ActiveWaterBeamFX))
	{
		ActiveWaterBeamFX->Destroy();
	}

	ActiveWaterBeamFX = nullptr;
}

FVector UGideon_Skill1A_WaterCannon::ResolveFXStartLocation() const
{
	const ATP_Character* C = GetOwnerChar();
	if (!C)
	{
		return FVector::ZeroVector;
	}

	const USkeletalMeshComponent* MeshComp = C->GetMesh();

	if (MeshComp && C->Skill1A_StartSocket != NAME_None && MeshComp->DoesSocketExist(C->Skill1A_StartSocket))
	{
		return MeshComp->GetSocketLocation(C->Skill1A_StartSocket);
	}

	return C->GetActorLocation() + C->GetActorForwardVector() * 50.f + FVector(0.f, 0.f, 80.f);
}

FVector UGideon_Skill1A_WaterCannon::ResolveFXEndLocation(const FVector& StartLocation) const
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

	return StartLocation + Direction.GetSafeNormal() * C->Skill1A_Range;
}