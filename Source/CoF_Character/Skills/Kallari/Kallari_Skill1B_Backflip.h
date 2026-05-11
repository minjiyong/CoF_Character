#pragma once

#include "CoreMinimal.h"
#include "Skills/Terra/Terra_Skill1A_Dash.h"
#include "Kallari_Skill1B_Backflip.generated.h"

/**
 * Kallari Skill1_B 공중제비 회피
 * - Terra_Skill1A_Dash 로직을 최대한 재사용
 * - 차이점은 뒤 + 위 방향 이동, 공격 없음
 */
UCLASS()
class COF_CHARACTER_API UKallari_Skill1B_Backflip : public UTerra_Skill1A_Dash
{
    GENERATED_BODY()

public:
    void DashStart();
    void DashEnd();
};