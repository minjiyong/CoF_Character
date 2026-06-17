#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ShijuBasicArrow.generated.h"

UCLASS()
class COF_CHARACTER_API UBTTask_ShijuBasicArrow : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ShijuBasicArrow();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};