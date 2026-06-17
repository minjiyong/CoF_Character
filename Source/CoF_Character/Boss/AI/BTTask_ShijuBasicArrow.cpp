#include "BTTask_ShijuBasicArrow.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Shiju/ShijuBoss.h"

UBTTask_ShijuBasicArrow::UBTTask_ShijuBasicArrow()
{
    NodeName = TEXT("Shiju Basic Arrow");
}

EBTNodeResult::Type UBTTask_ShijuBasicArrow::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Failed;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }

    Boss->SetCurrentTarget(Target);

    return Boss->PerformBasicArrowAttack()
        ? EBTNodeResult::Succeeded
        : EBTNodeResult::Failed;
}