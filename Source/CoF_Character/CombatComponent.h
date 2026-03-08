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
	void ConfigureTraceHit(float InDamage);

	// 스킬1 AOE용: 구형 광역 판정으로 설정
	void ConfigureAOEHit(float InDamage, float InRadius);


protected:
	bool DoLineTrace(FHitResult& OutHit) const;

	// HitWindow에서 실제로 1회 실행되는 판정 본체
	void ExecuteHitOnce();

	// HitReact 호출 유틸
	void ApplyHitToActor(AActor* Target, float InDamage, const FVector& HitPoint, const FVector& HitNormal);


private:
	bool bHitWindowOpen = false;
	bool bHitAppliedThisSwing = false;

	EHitQueryType HitQueryType = EHitQueryType::None;

	// “이번 판정” 파라미터
	float PendingDamage = 0.f;
	float PendingRadius = 0.f;
};
