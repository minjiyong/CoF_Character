#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Terra_UltA_AllyShield.generated.h"

/**
 * 궁_A 아군 쉴드
 * - 주변 Sphere 범위 내 Tag(Ally) 대상에게 보호막 부여
 * - 시간이 지나면 ShieldEnd가 호출되고 보호막이 제거됨.
 */

UCLASS()
class COF_CHARACTER_API UTerra_UltA_AllyShield : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	void ShieldStart(); // 몽타주 Notify에서 호출 (부여)
	void ShieldEnd();   // 타이머에서 자동 호출 (제거)
};