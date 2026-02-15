#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitReactInterface.generated.h"

UINTERFACE(BlueprintType)
class COF_CHARACTER_API UHitReactInterface : public UInterface
{
	GENERATED_BODY()
};

class COF_CHARACTER_API IHitReactInterface
{
	GENERATED_BODY()

public:
	// 맞았을 때 호출: BP에서도 구현 가능
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HitReact")
	void OnHitReact(float DamageAmount, const FVector& HitPoint, const FVector& HitNormal);
};
