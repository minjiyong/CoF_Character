#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Gideon_UltB_WaterBombDrop.generated.h"

class ACoF_CommonProjectile;

UCLASS()
class COF_CHARACTER_API UGideon_UltB_WaterBombDrop : public UCoF_SkillBase
{
	GENERATED_BODY()

public:
	void ResetRuntime();

	bool IsInCooldown(double Now) const;
	void StartCooldown(double Now, float Cooldown);

	// 몽타주 Notify에서 호출되는 실제 물폭탄 낙하 시작 함수
	void DropStart();

	// 물폭탄 Actor가 지점에 도달했을 때 호출하는 폭발 처리 함수
	void ExplodeAtLocation(const FVector& ImpactLocation);

private:
	double CooldownEndTime = 0.0;

	UPROPERTY()
	TObjectPtr<ACoF_CommonProjectile> ActiveBomb = nullptr;

	UWorld* GetWorldFromOwner() const;

	FVector ResolveImpactLocation() const;
	bool ProjectToGround(const FVector& SourceLocation, FVector& OutGroundLocation) const;
};