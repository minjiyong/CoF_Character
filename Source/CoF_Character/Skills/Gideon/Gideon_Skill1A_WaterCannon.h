#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_Skill1A_WaterCannon.generated.h"

UCLASS()
class COF_CHARACTER_API UGideon_Skill1A_WaterCannon : public UCoF_SkillBase
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

	void HitStart();
	void HitEnd();
};