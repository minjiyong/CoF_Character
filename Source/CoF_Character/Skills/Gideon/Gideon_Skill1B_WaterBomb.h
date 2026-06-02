#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_Skill1B_WaterBomb.generated.h"

class ACoF_CommonProjectile;

UCLASS()
class COF_CHARACTER_API UGideon_Skill1B_WaterBomb : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;
	bool bExplosionConsumed = false;

	UPROPERTY()
	TObjectPtr<ACoF_CommonProjectile> ActiveProjectile = nullptr;

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