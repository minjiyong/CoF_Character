#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_UltA_MirrorWaterBeam.generated.h"

UCLASS()
class COF_CHARACTER_API UGideon_UltA_MirrorWaterBeam : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;
	bool bBeamActive = false;

	FTimerHandle BeamTickTimerHandle;
	FTimerHandle BeamEndTimerHandle;

	void ResetRuntime()
	{
		NextAvailableTime = 0.0;
		bBeamActive = false;

		if (UWorld* W = GetWorld())
		{
			W->GetTimerManager().ClearTimer(BeamTickTimerHandle);
			W->GetTimerManager().ClearTimer(BeamEndTimerHandle);
		}
	}

	bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
	void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

	void BeamStart();
	void BeamTick();
	void BeamEnd();

private:
	UWorld* GetWorld() const;
};