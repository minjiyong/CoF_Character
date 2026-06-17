#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ShijuQTimeRain.generated.h"

UCLASS()
class COF_CHARACTER_API UBTTask_ShijuQTimeRain : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ShijuQTimeRain();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};