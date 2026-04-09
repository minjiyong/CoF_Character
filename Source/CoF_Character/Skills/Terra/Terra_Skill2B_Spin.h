#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Terra_Skill2B_Spin.generated.h"

/**
 * 스킬2_B 돌기
 * - HitStart : Spin 판정 시작
 * - SpinEnd  : 돌기 시간 끝나면 End 섹션으로 전환
 */

UCLASS()
class COF_CHARACTER_API UTerra_Skill2B_Spin : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	void HitStart();
	void SpinEnd(); // 돌기 시간 끝나면 End로
};