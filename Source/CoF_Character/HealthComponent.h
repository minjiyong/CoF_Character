#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpChanged, float, NewHp, float, Delta);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COF_CHARACTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHp = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHp = 100.f;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHpChanged OnHpChanged;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHp();

	// 로컬 데미지 적용(지금 단계)
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage_Local(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const { return CurrentHp <= 0.f; }
};
