#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DebuffBallTargetInterface.generated.h"

UINTERFACE(BlueprintType)
class COF_CHARACTER_API UDebuffBallTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class COF_CHARACTER_API IDebuffBallTargetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "DebuffBall")
	void ApplyDebuffBall(float InDuration, float InIncomingDamageMultiplier);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "DebuffBall")
	bool IsDebuffBallActive() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "DebuffBall")
	float GetDebuffBallIncomingDamageMultiplier() const;
};