#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHpChanged,
	float,
	NewHp,
	float,
	Delta
);

// 체력, 최대 체력, 보호막 중 하나라도 변경됐을 때 HUD에 전달한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnHealthStatusChanged,
	float,
	CurrentHp,
	float,
	MaxHp,
	float,
	Shield
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COF_CHARACTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHp = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHp = 1000.f;

	// 기존 체력 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHpChanged OnHpChanged;

	// 체력 및 보호막 HUD 갱신용 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthStatusChanged OnHealthStatusChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float Shield = 0.f; // 보호막(HP보다 먼저 깎임)

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHp();

	// 로컬 데미지 적용(지금 단계)
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage_Local(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const
	{
		return CurrentHp <= 0.f;
	}

	// 추가 보호막
	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddShield(float Bonus);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void RemoveShield(float Bonus);

private:
	// 현재 체력과 보호막 상태를 HUD에 전달한다.
	void BroadcastHealthStatus();
};