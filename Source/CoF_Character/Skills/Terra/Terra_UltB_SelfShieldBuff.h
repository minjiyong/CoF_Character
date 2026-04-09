#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Terra_UltB_SelfShieldBuff.generated.h"

/**
 * 궁_B 버프
 * - 자체 보호막 + 공격력 배율 증가
 * - 시간이 지나면 BuffEnd가 호출되고 보호막이 제거됨.
 */

UCLASS()
class COF_CHARACTER_API UTerra_UltB_SelfShieldBuff : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;

	bool bActive = false;			// 런타임 버프 상태
	FTimerHandle EndTimerHandle;	// 런타임 버프 상태

	void ResetRuntime()
	{
		NextAvailableTime = 0.0;
		bActive = false;
		if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(EndTimerHandle);
	}

	bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
	void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

private:
	UWorld* GetWorld() const;

public:
	void BuffStart();
	void BuffEnd();
};