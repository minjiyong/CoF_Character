#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Terra_Skill1A_Dash.generated.h"

/**
 * Skill1_A 돌진
 * - TP_Character.cpp의 Skill1A_* 관련 로직을 그대로 옮긴 파일
 */

UCLASS()
class COF_CHARACTER_API UTerra_Skill1A_Dash : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	void HitStart();
	void HitEnd();			// 역시나 당장은 필요없는듯 기존 hitend 돌려쓰는중 나중에 필요하면 바꾸자.			
	void DashStart();		// 실제 돌진 시, 상태를 fly로 만듬(바닥 충돌 때문에)
	void DashEnd();
};