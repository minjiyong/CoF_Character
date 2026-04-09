#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Terra_Skill2B_Spin.generated.h"

/**
 * 스킬2_B 돌기
 * - HitStart : Spin 판정 시작
 * - SpinEnd  : 돌기 시간 끝나면 End 섹션으로 전환
 */

UCLASS()
class COF_CHARACTER_API UTerra_Skill2B_Spin : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	// ===== runtime state (TP_Character에서 옮겨옴) =====
	double NextAvailableTime = 0.0;

	bool bActive = false;
	FTimerHandle EndTimerHandle;
	double EndTime = 0.0; // 스킬 시전한 후 남은 시간

	void ResetRuntime()
	{
		NextAvailableTime = 0.0;
		bActive = false;
		EndTime = 0.0;
		if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(EndTimerHandle);
	}

	bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
	void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

	bool IsActive() const { return bActive; }

	// 타이머 예약까지 스킬이 직접 처리(TP_Character에서 제거)
	void BeginSpin(double Now, float DurationSec);
	void CancelSpin();

private:
	UWorld* GetWorld() const;

public:
	void HitStart();
	void SpinEnd(); // 돌기 시간 끝나면 End로
};