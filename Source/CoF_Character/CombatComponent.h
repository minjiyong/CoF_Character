#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UENUM()
enum class EHitQueryType : uint8
{
	None,
	TraceForward,
	AOESphere,
	DashTrace,
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COF_CHARACTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float TraceRange = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bDrawDebug = true;

	// 공격
	void BeginHitWindow_OneShot();
	void EndHitWindow();

	// 콤보/기본공격용: 라인트레이스 판정으로 설정
	void ConfigureTraceHit(float InDamage, float InRange);

	// 스킬 1_A 돌진용
	void ConfigureDashHit(float InDamage, float InRange, float InDuration);			// duration으로 기간 동안 반복 판정

	// 스킬1_B AOE용: 구형 광역 판정으로 설정
	void ConfigureAOEHit(float InDamage, float InRadius);


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool DoLineTrace(FHitResult& OutHit) const;
	bool DoLineTraceWithRange(FHitResult& OutHit, float InRange) const;

	// HitWindow에서 실제로 1회 실행되는 판정 본체, 기본공격용
	void ExecuteHitOnce();

	// HitReact 호출 유틸
	void ApplyHitToActor(AActor* Target, float InDamage, const FVector& HitPoint, const FVector& HitNormal);


private:
	bool bHitWindowOpen = false;
	bool bHitAppliedThisSwing = false;

	EHitQueryType HitQueryType = EHitQueryType::None;

	// 이번 공격 판정 파라미터
	float PendingDamage = 0.f;
	// 기본공격은 x
	float PendingRadius = 0.f;
	float PendingRange = 0.f;

	// Dash 전용
	double DashEndTime = 0.0;
	TSet<TWeakObjectPtr<AActor>> DashHitActors;
};
