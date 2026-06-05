#pragma once

#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_UltB_WaterBombDrop.generated.h"

class ACoF_CommonProjectile;
class AGideon_UltB_WaterBombActor;

UCLASS()
class COF_CHARACTER_API UGideon_UltB_WaterBombDrop : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	double NextAvailableTime = 0.0;
	bool bDropActive = false;

	UPROPERTY()
	TObjectPtr<AGideon_UltB_WaterBombActor> ActiveBomb = nullptr;

	// 궁B 런타임 상태를 초기화한다.
	void ResetRuntime();

	// 현재 시간이 쿨타임 중인지 확인한다.
	bool IsInCooldown(double Now) const;

	// 궁B 쿨타임을 시작한다.
	void StartCooldown(double Now, float CooldownSec);

	// 몽타주 Notify에서 호출된다. 목표 위치를 정하고 물폭탄 연출용 액터를 생성한다.
	void DropStart();

	// 물폭탄 낙하가 끝났을 때 호출된다. 착탄 위치에 AOE 데미지를 적용한다.
	void ExplodeAtLocation(const FVector& ImpactLocation);

private:
	UWorld* GetWorld() const;

	// 락온 대상 또는 캐릭터 전방 기준으로 최종 착탄 위치를 계산한다.
	bool ResolveImpactLocation(FVector& OutImpactLocation) const;

	// 기준 위치에서 아래 방향으로 Trace하여 바닥 위치를 찾는다.
	bool ProjectToGround(const FVector& SourceLocation, FVector& OutGroundLocation) const;
};