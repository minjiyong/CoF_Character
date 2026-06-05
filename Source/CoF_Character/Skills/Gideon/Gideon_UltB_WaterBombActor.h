#pragma once

#include "CoreMinimal.h"
#include "Projectiles/CoF_CommonProjectile.h"
#include "Gideon_UltB_WaterBombActor.generated.h"

class UGideon_UltB_WaterBombDrop;

UCLASS()
class COF_CHARACTER_API AGideon_UltB_WaterBombActor : public ACoF_CommonProjectile
{
	GENERATED_BODY()

public:
	AGideon_UltB_WaterBombActor();

	virtual void Tick(float DeltaSeconds) override;

	// 물폭탄 연출용 액터를 낙하 시작 위치와 착탄 위치 기준으로 초기화한다.
	void InitVisualBomb(
		UGideon_UltB_WaterBombDrop* InOwningSkill,
		const FVector& InStartLocation,
		const FVector& InImpactLocation,
		float InFallDuration
	);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Skills|Ult|B|Gideon|Visual")
	void BP_OnBombInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "Skills|Ult|B|Gideon|Visual")
	void BP_OnBombExploded();

private:
	UPROPERTY()
	TObjectPtr<UGideon_UltB_WaterBombDrop> WaterBombSkill = nullptr;

	FVector StartLocation = FVector::ZeroVector;
	FVector ImpactLocation = FVector::ZeroVector;

	float FallDuration = 0.8f;
	float ElapsedTime = 0.0f;

	bool bInitialized = false;
	bool bExploded = false;

	// 낙하가 끝났을 때 스킬 객체에 폭발 처리를 요청하고 자신을 제거한다.
	void FinishDrop();
};