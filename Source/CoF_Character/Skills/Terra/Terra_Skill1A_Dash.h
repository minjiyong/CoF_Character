#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Skills/CoF_SkillBase.h"
#include "Terra_Skill1A_Dash.generated.h"

/**
 * Skill1_A 돌진
 * - TP_Character.cpp의 Skill1A_* 관련 로직을 그대로 옮긴 파일
 */
UCLASS()
class COF_CHARACTER_API UTerra_Skill1A_Dash : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	// ===== runtime state (TP_Character에서 옮겨옴) =====
	double NextAvailableTime = 0.0;

	// 대쉬 값 저장 후 원복용
	bool bDashMoving = false; // 돌진 중인지. 다른 키입력 방지용
	float SavedGroundFriction = 0.f;
	float SavedBrakingFrictionFactor = 0.f;
	float SavedBrakingDecelerationWalking = 0.f;
	float SavedBrakingDecelerationFlying = 0.f;

	bool bSavedOrientRotationToMovement = false;
	bool bSavedUseControllerRotationYaw = false;

	// 실제 거리 기반 dash 보간용
	FTimerHandle DashMoveTimerHandle;
	FVector DashStartLocation = FVector::ZeroVector;
	FVector DashTargetLocation = FVector::ZeroVector;
	double DashStartTime = 0.0;
	float DashMoveDuration = 0.f;
	TEnumAsByte<EMovementMode> DashStartMovementMode = MOVE_Flying;
	TEnumAsByte<EMovementMode> DashEndMovementMode = MOVE_Walking;
	bool bDashUseSweep = true;

	// 런타임 초기화(캐릭터 교체/데이터 교체 시 호출)
	void ResetRuntime();

	// 쿨다운 체크/시작
	bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
	void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

public:
	void HitStart();
	void HitEnd(); // 역시나 당장은 필요없는듯 기존 hitend 돌려쓰는중 나중에 필요하면 바꾸자.

	void DashStart(); // 실제 돌진 시, 상태를 fly로 만듬(바닥 충돌 때문에)
	void DashEnd();

protected:
	// 거리 기반 dash 공통 helper
	void BeginDashMoveToOffset(
		const FVector& WorldOffset,
		float InDuration,
		EMovementMode InStartMode,
		EMovementMode InEndMode,
		bool bInUseSweep = true);

	void UpdateDashMove();
	void FinishDashMove(bool bForceToTarget);
};