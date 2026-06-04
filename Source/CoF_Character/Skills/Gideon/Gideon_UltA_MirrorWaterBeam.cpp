#include "Skills/Gideon/Gideon_UltA_MirrorWaterBeam.h"

#include "CombatComponent.h"
#include "TP_Character.h"
#include "TimerManager.h"

UWorld* UGideon_UltA_MirrorWaterBeam::GetWorld() const
{
	if (ATP_Character* C = GetOwnerChar())
	{
		return C->GetWorld();
	}
	return nullptr;
}

void UGideon_UltA_MirrorWaterBeam::BeamStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;
	if (!C->CombatComp) return;
	if (bBeamActive) return;

	UWorld* W = GetWorld();
	if (!W) return;

	bBeamActive = true;

	C->SetMoveInputEnabled(false);
	C->SetAttackInputEnabled(false);
	C->SetGuardInputEnabled(false);
	C->SetSkillInputEnabled(false);
	C->SetJumpInputEnabled(false);

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
	if (!C) return;
	if (!C->CombatComp) return;
	if (!bBeamActive) return;

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
	if (!C) return;

	UWorld* W = GetWorld();
	if (W)
	{
		W->GetTimerManager().ClearTimer(BeamTickTimerHandle);
		W->GetTimerManager().ClearTimer(BeamEndTimerHandle);
	}

	bBeamActive = false;
	C->SetEveryInputEnabled(true);
}