#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_Skill1B_WaterBomb.generated.h"

class AKallari_Skill2A_ShurikenProjectile;

UCLASS()
class COF_CHARACTER_API UGideon_Skill1B_WaterBomb : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;
	bool bExplosionConsumed = false;

	UPROPERTY()
	TObjectPtr<AKallari_Skill2A_ShurikenProjectile> ActiveProjectile = nullptr;

	void ResetRuntime()
	{
		NextAvailableTime = 0.0;
		bExplosionConsumed = false;
		ActiveProjectile = nullptr;
	}

	bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
	void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

	void ThrowProjectile();
	void ExplodeAtLocation(const FVector& ImpactLocation);
};