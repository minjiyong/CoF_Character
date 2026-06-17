#include "BTTask_ShijuQTimeRain.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Shiju/ShijuBoss.h"

UBTTask_ShijuQTimeRain::UBTTask_ShijuQTimeRain()
{
    NodeName = TEXT("Shiju Q Time Rain");
}

EBTNodeResult::Type UBTTask_ShijuQTimeRain::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    return Boss->PerformQTimeRain()
        ? EBTNodeResult::Succeeded
        : EBTNodeResult::Failed;
}