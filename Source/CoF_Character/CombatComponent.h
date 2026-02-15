#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UInputAction;
struct FInputActionValue;

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

	// 로컬 공격(서버 없는 버전)
	void TryAttack_Local();

protected:
	bool DoLineTrace(FHitResult& OutHit) const;
};
