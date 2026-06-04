#pragma once

#include "CoreMinimal.h"
#include "Skills/Terra/Terra_Skill1A_Dash.h"
#include "Gideon_Skill2B_BackDash.generated.h"

/**
 * Gideon Skill2_B 백대쉬
 * - 이동만 담당
 * - Terra_Skill1A_Dash 의 거리 기반 dash helper 재사용
 */
UCLASS()
class COF_CHARACTER_API UGideon_Skill2B_BackDash : public UTerra_Skill1A_Dash
{
	GENERATED_BODY()

public:
	void DashStart();
	void DashEnd();
};