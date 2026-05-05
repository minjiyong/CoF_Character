#pragma once

#include "CoreMinimal.h"
#include "Skills/Terra/Terra_Skill1A_Dash.h"
#include "Kallari_Skill1A_DashSlash.generated.h"

/**
 * Kallari Skill1_A 질풍참 스타일 돌진
 * - Terra_Skill1A_Dash를 최대한 재사용
 */
UCLASS()
class COF_CHARACTER_API UKallari_Skill1A_DashSlash : public UTerra_Skill1A_Dash
{
    GENERATED_BODY()

public:
    void HitStart();
    void DashStart();
};