#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Terra_Skill1B_SlamAOE.generated.h"

/**
 * skill1_B 적용 AOE(광역 공격)
 * - TP_Character.cpp의 Skill1B_ApplyAOE() 로직 분리
 */

UCLASS()
class COF_CHARACTER_API UTerra_Skill1B_SlamAOE : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	void ApplyAOE();
};