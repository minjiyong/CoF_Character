#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "TimerManager.h"
#include "Gideon_Skill2A_DebuffBall.generated.h"

class AActor;
class ACoF_CommonProjectile;

UCLASS()
class COF_CHARACTER_API UGideon_Skill2A_DebuffBall
	: public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;

	void ResetRuntime();

	bool IsInCooldown(double Now) const
	{
		return Now < NextAvailableTime;
	}

	void StartCooldown(double Now, float CooldownSec)
	{
		NextAvailableTime = Now + CooldownSec;
	}

	void ThrowProjectile();

	// 투사체가 적중한 대상에게 디버프와 FX를 적용한다.
	void ApplyDebuffToActor(AActor* Target);

private:
	// 대상에게 디버프 FX를 생성한다.
	// 이미 FX가 있다면 새로 만들지 않고 제거 타이머만 갱신한다.
	void SpawnOrRefreshDebuffFX(AActor* Target);

	// 특정 대상에게 붙은 디버프 FX를 제거한다.
	void RemoveDebuffFX(TWeakObjectPtr<AActor> TargetKey);

	// 현재 남아 있는 모든 디버프 FX와 타이머를 제거한다.
	void ClearAllDebuffFX();

	// 대상별로 생성된 디버프 FX Actor
	TMap<
		TWeakObjectPtr<AActor>,
		TWeakObjectPtr<AActor>
	> DebuffFXByTarget;

	// 대상별 FX 제거 타이머
	TMap<
		TWeakObjectPtr<AActor>,
		FTimerHandle
	> DebuffFXTimerByTarget;
};