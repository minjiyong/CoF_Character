#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CoF_SkillBase.generated.h"

class ATP_Character;

/**
 * 스킬 분리 베이스
 * - ATP_Character(플레이어 캐릭터) 안의 기존 변수/함수명을 그대로 사용하기 위해
 *   Owner 캐릭터 포인터를 들고, 각 Terra_* 스킬 객체가 TP_Character의 데이터를 그대로 참조한다.
 */

UCLASS(Abstract)
class COF_CHARACTER_API UCoF_SkillBase : public UObject
{
	GENERATED_BODY()

public:
	// 스킬 객체가 어떤 캐릭터에 붙어있는지 초기화
	void Init(ATP_Character* InOwner) { Owner = InOwner; }

protected:
	ATP_Character* GetOwnerChar() const { return Owner.Get(); }

private:
	TObjectPtr<ATP_Character> Owner;
};