#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShijuAnimInstance.generated.h"

UCLASS()
class COF_CHARACTER_API UShijuAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

protected:
    UFUNCTION()
    void AnimNotify_P_Fire();
};