#include "BTTask_ShijuRSkill.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "../Shiju/ShijuBoss.h"

UBTTask_ShijuRSkill::UBTTask_ShijuRSkill()
{
    NodeName = TEXT("Shiju R Skill");
}

EBTNodeResult::Type UBTTask_ShijuRSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    AShijuBoss* Boss = Cast<AShijuBoss>(AIController->GetPawn());
    if (!Boss)
    {
        return EBTNodeResult::Failed;
    }

    return Boss->PerformRSkill() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}