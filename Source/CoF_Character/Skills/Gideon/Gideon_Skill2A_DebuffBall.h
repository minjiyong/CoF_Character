#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_Skill2A_DebuffBall.generated.h"

class ACoF_CommonProjectile;

UCLASS()
class COF_CHARACTER_API UGideon_Skill2A_DebuffBall : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;

	void ResetRuntime()
	{
		NextAvailableTime = 0.0;
	}

	bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
	void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

	void ThrowProjectile();
	void ApplyDebuffToActor(AActor* Target);
};