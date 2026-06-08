#pragma once
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ShijuUsePattern.generated.h"

UCLASS()
class COF_CHARACTER_API UBTTask_ShijuUsePattern : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ShijuUsePattern();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};