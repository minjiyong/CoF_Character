#pragma once

#include "CoreMinimal.h"
#include "Skills/Terra/Terra_Skill1A_Dash.h"
#include "Kallari_Skill1B_RisingDashSlash.generated.h"

/**
 * Kallari Skill1_B 상승 돌진 공격
 * - Terra_Skill1A_Dash 활용
 * - 차이점은 전방 돌진 대신 위로 상승
 */
UCLASS()
class COF_CHARACTER_API UKallari_Skill1B_RisingDashSlash : public UTerra_Skill1A_Dash
{
    GENERATED_BODY()

public:
    void HitStart();
    void DashStart();
};