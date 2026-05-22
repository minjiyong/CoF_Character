#pragma once

#include "CoreMinimal.h"
#include "Skills/Terra/Terra_Skill1A_Dash.h"
#include "Kallari_UltA_BlinkDash.generated.h"

/**
 * Kallari 궁극기 A
 * - 매우 빠른 전방 돌진 공격
 * - 기존 Terra dash 로직 재사용
 */
UCLASS()
class COF_CHARACTER_API UKallari_UltA_BlinkDash : public UTerra_Skill1A_Dash
{
    GENERATED_BODY()

public:
    void HitStart();
    void DashStart();
    void DashEnd();
};