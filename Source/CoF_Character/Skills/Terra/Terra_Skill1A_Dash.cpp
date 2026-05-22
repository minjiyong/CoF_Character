#include "Skills/Terra/Terra_Skill1A_Dash.h"

#include "CombatComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TP_Character.h"

void UTerra_Skill1A_Dash::ResetRuntime()
{
	if (ATP_Character* C = GetOwnerChar())
	{
		if (UWorld* World = C->GetWorld())
		{
			World->GetTimerManager().ClearTimer(DashMoveTimerHandle);
		}
	}

	NextAvailableTime = 0.0;
	bDashMoving = false;
	DashStartLocation = FVector::ZeroVector;
	DashTargetLocation = FVector::ZeroVector;
	DashStartTime = 0.0;
	DashMoveDuration = 0.f;
}

void UTerra_Skill1A_Dash::HitStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;

	C->CombatComp->ConfigureDashHit(
		C->Skill1A_Damage * C->AttackMultiplier,
		C->Skill1A_DashDuration,
		C->Skill1A_HitRadius
	);

	C->CombatComp->BeginHitWindow_OneShot();
}

void UTerra_Skill1A_Dash::HitEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;

	C->CombatComp->EndHitWindow();
}

void UTerra_Skill1A_Dash::BeginDashMoveToOffset(
	const FVector& WorldOffset,
	float InDuration,
	EMovementMode InStartMode,
	EMovementMode InEndMode,
	bool bInUseSweep)
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	UCharacterMovementComponent* MoveComp = C->GetCharacterMovement();
	if (!MoveComp || bDashMoving) return;

	UWorld* World = C->GetWorld();
	if (!World) return;

	bDashMoving = true;

	// 기존 저장값
	SavedGroundFriction = MoveComp->GroundFriction;
	SavedBrakingFrictionFactor = MoveComp->BrakingFrictionFactor;
	SavedBrakingDecelerationWalking = MoveComp->BrakingDecelerationWalking;
	SavedBrakingDecelerationFlying = MoveComp->BrakingDecelerationFlying;
	bSavedOrientRotationToMovement = MoveComp->bOrientRotationToMovement;
	bSavedUseControllerRotationYaw = C->bUseControllerRotationYaw;

	// dash 동안 마찰/감속 제거
	MoveComp->GroundFriction = 0.f;
	MoveComp->BrakingFrictionFactor = 0.f;
	MoveComp->BrakingDecelerationWalking = 0.f;
	MoveComp->BrakingDecelerationFlying = 0.f;
	MoveComp->bOrientRotationToMovement = false;
	C->bUseControllerRotationYaw = false;

	MoveComp->StopMovementImmediately();
	MoveComp->SetMovementMode(InStartMode);

	DashStartMovementMode = InStartMode;
	DashEndMovementMode = InEndMode;
	bDashUseSweep = bInUseSweep;

	DashStartLocation = C->GetActorLocation();
	DashTargetLocation = DashStartLocation + WorldOffset;
	DashMoveDuration = FMath::Max(InDuration, 0.01f);
	DashStartTime = World->GetTimeSeconds();

	World->GetTimerManager().ClearTimer(DashMoveTimerHandle);
	World->GetTimerManager().SetTimer(
		DashMoveTimerHandle,
		this,
		&UTerra_Skill1A_Dash::UpdateDashMove,
		0.005f,
		true
	);

	UpdateDashMove();
}

void UTerra_Skill1A_Dash::UpdateDashMove()
{
	ATP_Character* C = GetOwnerChar();
	if (!C)
	{
		FinishDashMove(false);
		return;
	}

	UWorld* World = C->GetWorld();
	if (!World || !bDashMoving)
	{
		FinishDashMove(false);
		return;
	}

	const float Alpha = FMath::Clamp(
		static_cast<float>((World->GetTimeSeconds() - DashStartTime) / DashMoveDuration),
		0.f,
		1.f
	);

	const FVector NewLocation = FMath::Lerp(DashStartLocation, DashTargetLocation, Alpha);

	FHitResult SweepHit;
	C->SetActorLocation(NewLocation, bDashUseSweep, &SweepHit);

	// 벽에 막히면 현재 위치에서 종료
	if (SweepHit.bBlockingHit)
	{
		FinishDashMove(false);
		return;
	}

	// 목표 거리 도달
	if (Alpha >= 1.f - KINDA_SMALL_NUMBER)
	{
		FinishDashMove(true);
	}
}

void UTerra_Skill1A_Dash::FinishDashMove(bool bForceToTarget)
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	UWorld* World = C->GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(DashMoveTimerHandle);
	}

	UCharacterMovementComponent* MoveComp = C->GetCharacterMovement();
	if (!MoveComp)
	{
		bDashMoving = false;
		return;
	}

	if (bForceToTarget)
	{
		FHitResult SweepHit;
		C->SetActorLocation(DashTargetLocation, bDashUseSweep, &SweepHit);
	}

	bDashMoving = false;

	MoveComp->StopMovementImmediately();
	MoveComp->SetMovementMode(DashEndMovementMode);

	// 복구
	MoveComp->GroundFriction = SavedGroundFriction;
	MoveComp->BrakingFrictionFactor = SavedBrakingFrictionFactor;
	MoveComp->BrakingDecelerationWalking = SavedBrakingDecelerationWalking;
	MoveComp->BrakingDecelerationFlying = SavedBrakingDecelerationFlying;
	MoveComp->bOrientRotationToMovement = bSavedOrientRotationToMovement;
	C->bUseControllerRotationYaw = bSavedUseControllerRotationYaw;
	MoveComp->UpdateComponentVelocity();
}

void UTerra_Skill1A_Dash::DashStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	const FVector ForwardOffset =
		C->GetActorForwardVector().GetSafeNormal2D() * C->Skill1A_DashDistance;

	BeginDashMoveToOffset(
		ForwardOffset,
		C->Skill1A_DashDuration,
		MOVE_Flying,
		MOVE_Walking,
		true
	);
}

void UTerra_Skill1A_Dash::DashEnd()
{
	// notify가 살짝 빠르거나 늦어도 최종 목표 지점에서 정리
	FinishDashMove(true);
}