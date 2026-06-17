#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_Skill1A_WaterCannon.generated.h"

class AActor;

UCLASS()
class COF_CHARACTER_API UGideon_Skill1A_WaterCannon : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;

	void ResetRuntime();

	bool IsInCooldown(double Now) const;
	void StartCooldown(double Now, float CooldownSec);

	// Gideon Skill1A 판정과 물줄기 이펙트를 시작한다.
	void HitStart();

	// Gideon Skill1A 판정과 물줄기 이펙트를 종료한다.
	void HitEnd();

private:
	UPROPERTY()
	TObjectPtr<AActor> ActiveWaterBeamFX = nullptr;

	void SpawnWaterBeamFX();
	void ClearWaterBeamFX();

	FVector ResolveFXStartLocation() const;
	FVector ResolveFXEndLocation(const FVector& StartLocation) const;
};