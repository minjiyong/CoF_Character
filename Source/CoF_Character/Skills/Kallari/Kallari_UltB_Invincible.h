#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Kallari_UltB_Invincible.generated.h"

/**
 * Kallari 궁_B 무적 버프
 * - 시전 후 일정 시간 동안 피격 무시
 * - 시간이 지나면 BuffEnd가 호출됨.
 */
UCLASS()
class COF_CHARACTER_API UKallari_UltB_Invincible : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;
	bool bActive = false;              // 런타임 무적 상태
	FTimerHandle EndTimerHandle;       // 런타임 종료 타이머

	void ResetRuntime()
	{
		NextAvailableTime = 0.0;
		bActive = false;
		if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(EndTimerHandle);
	}

	bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
	void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

	bool IsActive() const { return bActive; }

private:
	UWorld* GetWorld() const;

public:
	void BuffStart();
	void BuffEnd();
};