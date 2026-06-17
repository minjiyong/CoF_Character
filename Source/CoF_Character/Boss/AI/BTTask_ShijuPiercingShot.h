#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ShijuPiercingShot.generated.h"

UCLASS()
class COF_CHARACTER_API UBTTask_ShijuPiercingShot : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ShijuPiercingShot();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};