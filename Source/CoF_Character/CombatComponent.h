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
	SpinSweep,
	AOEForward,
	AOELocation,
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
	void ConfigureDashHit(float InDamage, float InDuration, float InRadius);			// duration으로 기간 동안 반복 판정

	// 스킬1_B AOE용: 구형 광역 판정으로 설정, 플레이어 몸 주변
	void ConfigureAOEHit(float InDamage, float InRadius);
	
	// 스킬 2_A 전방 광역(원뿔/부채꼴) 설정
	void ConfigureAOEForwardHit(float InDamage, float InRadius, float InForwardOffset, float InHalfAngleDeg);

	// Kallari 스킬 2_B 위치 기반 단발성 광역 공격 설정
	void ConfigureAOELocationHit(const FVector& InCenter, float InDamage, float InRadius);

	// 스킬 2_B 돌기
	void ConfigureSpinHit(float InDamagePerTick, float InRadius, float InTickInterval, float InDuration);

	// HitReact 호출 유틸
	void ApplyHitToActor(AActor* Target, float InDamage, const FVector& HitPoint, const FVector& HitNormal);


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool DoLineTrace(FHitResult& OutHit) const;
	bool DoLineTraceWithRange(FHitResult& OutHit, float InRange) const;

	// HitWindow에서 실제로 1회 실행되는 판정 본체, 기본공격용
	void ProcessHitQuery();

private:
	bool bHitWindowOpen = false;
	bool bHitAppliedThisSwing = false;

	EHitQueryType HitQueryType = EHitQueryType::None;

	// 이번 공격 판정 파라미터
	float PendingDamage = 0.f;
	float PendingRange = 0.f;
	
	float PendingRadius = 0.f; // 기본공격은 x

	// Dash 전용
	double DashEndTime = 0.0;
	TSet<TWeakObjectPtr<AActor>> DashHitActors;
	float DashRadius = 60.f;         // 플레이어 몸통 크기 정도(튜닝)
	FVector DashPrevLoc = FVector::ZeroVector;

	// Spin 전용
	float SpinDamagePerTick = 0.f;
	float SpinRadius = 0.f;
	float SpinTickInterval = 0.2f;
	double SpinEndTime = 0.0;

	FVector SpinPrevLoc = FVector::ZeroVector;
	TMap<TWeakObjectPtr<AActor>, double> SpinLastHitTime; // 타겟별 틱 간격 관리

	// 전방 AOE 전용 파라미터
	float PendingForwardOffset = 0.f;
	float PendingHalfAngleDeg = 0.f;

	// 특정 위치 AOE 전용 파라미터
	FVector PendingAOELocation = FVector::ZeroVector;
};
