#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Terra_Skill2A_ShieldPush.generated.h"

/**
 * 스킬 2_A 방패 밀쳐내기 전방 광역 공격
 */

UCLASS()
class COF_CHARACTER_API UTerra_Skill2A_ShieldPush : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	void HitStart();
};