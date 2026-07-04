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

	// ¹°ÆøÅº ³«ÇÏ ½ÃÀÛ À§Ä¡¿Í µµÂø À§Ä¡¸¦ ¼³Á¤ÇÑ´Ù.
	void InitVisualBomb(
		UGideon_UltB_WaterBombDrop* InSkill,
		const FVector& InStartLocation,
		const FVector& InImpactLocation,
		float InFallDuration
	);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Skills|Ult|B|Gideon")
	void BP_OnBombInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "Skills|Ult|B|Gideon")
	void BP_OnBombExploded();

private:
	UPROPERTY()
	TObjectPtr<UGideon_UltB_WaterBombDrop> WaterBombSkill = nullptr;

	FVector StartLocation = FVector::ZeroVector;
	FVector ImpactLocation = FVector::ZeroVector;

	float FallDuration = 0.5f;
	float ElapsedTime = 0.f;

	bool bInitialized = false;
	bool bFinished = false;

	void FinishDrop();
};