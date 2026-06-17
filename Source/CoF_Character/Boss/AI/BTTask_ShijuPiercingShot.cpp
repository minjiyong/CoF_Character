#include "BTTask_ShijuPiercingShot.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Shiju/ShijuBoss.h"

UBTTask_ShijuPiercingShot::UBTTask_ShijuPiercingShot()
{
    NodeName = TEXT("Shiju Piercing Shot");
}

EBTNodeResult::Type UBTTask_ShijuPiercingShot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    return Boss->PerformPiercingShot()
        ? EBTNodeResult::Succeeded
        : EBTNodeResult::Failed;
}