#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_UltA_MirrorWaterBeam.generated.h"

class AActor;

UCLASS()
class COF_CHARACTER_API UGideon_UltA_MirrorWaterBeam : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;

	bool bBeamActive = false;

	FTimerHandle BeamTickTimerHandle;
	FTimerHandle BeamEndTimerHandle;

	void ResetRuntime();

	bool IsInCooldown(double Now) const
	{
		return Now < NextAvailableTime;
	}

	void StartCooldown(double Now, float CooldownSec)
	{
		NextAvailableTime = Now + CooldownSec;
	}

	// Gideon UltA 물광선 판정과 이펙트를 시작한다.
	void BeamStart();

	// Gideon UltA 물광선 판정을 반복 적용한다.
	void BeamTick();

	// Gideon UltA 물광선 판정과 이펙트를 종료한다.
	void BeamEnd();

private:
	UPROPERTY()
	TObjectPtr<AActor> ActiveWaterBeamFX = nullptr;

	UWorld* GetWorld() const;

	void SpawnWaterBeamFX();
	void ClearWaterBeamFX();

	FVector ResolveBeamFXStartLocation() const;
	FVector ResolveBeamFXEndLocation(const FVector& StartLocation) const;
};