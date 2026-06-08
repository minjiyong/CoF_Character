#include "BTTask_ShijuUsePattern.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Shiju/ShijuBoss.h"
#include "../Data/ShijuBossDataAsset.h"

UBTTask_ShijuUsePattern::UBTTask_ShijuUsePattern()
{
    NodeName = TEXT("Shiju Use Phase Pattern");
}

EBTNodeResult::Type UBTTask_ShijuUsePattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // 중요:
    // BasicArrow는 이동 중 발사 가능하게 만들 예정이므로
    // 공격 중이라고 여기서 StopMovement()를 호출하면 안 된다.
    if (Boss->IsAttackInProgress())
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

    UShijuBossDataAsset* Data = Boss->GetShijuData();
    if (!Data)
    {
        return EBTNodeResult::Failed;
    }

    const float DistanceToTarget = FVector::Dist2D(
        Boss->GetActorLocation(),
        Target->GetActorLocation()
    );

    const float CloseBasicArrowRange = Data->TooCloseRange + 150.f;

    // =========================
    // 가까운 거리 처리
    // =========================
    // 플레이어가 너무 가까우면 Piercing/Q/R 같은 큰 기술을 쓰지 않고
    // 뒤로 빠지는 중에도 BasicArrow만 사용한다.
    if (DistanceToTarget <= CloseBasicArrowRange)
    {
        Boss->PerformBasicArrowAttack();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Shiju UsePattern] Close Range -> BasicArrow | Distance = %.2f"),
            DistanceToTarget
        );

        return EBTNodeResult::Succeeded;
    }

    // 일반 거리에서는 기존 Phase별 패턴 선택 로직 사용
    return Boss->TryUsePhasePattern()
        ? EBTNodeResult::Succeeded
        : EBTNodeResult::Failed;
}