#include "BTTask_ShijuKeepRange.h"

#include "AIController.h"
#include "AITypes.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Navigation/PathFollowingComponent.h"
#include "../Shiju/ShijuBoss.h"
#include "../Data/ShijuBossDataAsset.h"

UBTTask_ShijuKeepRange::UBTTask_ShijuKeepRange()
{
    NodeName = TEXT("Shiju Keep Range");

    bCreateNodeInstance = true;

    bEnableCombatStrafe = true;
    CombatStrafeChance = 0.75f;
    CombatStrafeCooldown = 0.7f;
    CombatStrafeDistance = 420.f;
    CombatStrafeAcceptanceRadius = 120.f;
    CombatStrafeRadialCorrectionWeight = 0.65f;
    StrafeDirectionChangeChance = 0.25f;

    bAllowBasicArrowWhileRetreating = true;
    RetreatBasicArrowChance = 0.7f;

    bDrawBossRangeDebug = true;
    RangeDebugHeight = 8.f;

    LastCombatStrafeTime = -1000.f;
    StrafeDirectionSign = 1;
}

EBTNodeResult::Type UBTTask_ShijuKeepRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        return EBTNodeResult::Failed;
    }

    Boss->SetCurrentTarget(Target);

    UShijuBossDataAsset* Data = Boss->GetShijuData();
    if (!Data)
    {
        return EBTNodeResult::Failed;
    }

    DrawBossRangeDebug(Boss, Data);

    const FVector BossLocation = Boss->GetActorLocation();
    const FVector TargetLocation = Target->GetActorLocation();
    const float Distance = FVector::Dist2D(BossLocation, TargetLocation);

    AIController->SetFocus(Target, EAIFocusPriority::Gameplay);

    // 공격 중에는 KeepRange가 새 이동을 걸지 않는다.
    // StopMovement도 하지 않는다.
    if (Boss->IsAttackInProgress())
    {
        return EBTNodeResult::Failed;
    }

    // =========================
    // 1. 너무 가까우면 뒤로 빠진다.
    // 단, BasicArrow는 실행 가능하게 넘길 수 있다.
    // =========================
    if (Distance < Data->TooCloseRange)
    {
        FVector AwayDirection = BossLocation - TargetLocation;
        AwayDirection.Z = 0.f;
        AwayDirection = AwayDirection.GetSafeNormal();

        if (AwayDirection.IsNearlyZero())
        {
            AwayDirection = -Boss->GetActorForwardVector();
            AwayDirection.Z = 0.f;
            AwayDirection = AwayDirection.GetSafeNormal();
        }

        const float MoveDistance = FMath::Max(300.f, (Data->DesiredCombatRange - Distance) + 200.f);
        const FVector DesiredLocation = BossLocation + AwayDirection * MoveDistance;

        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(DesiredLocation);
        MoveRequest.SetAcceptanceRadius(120.f);
        MoveRequest.SetUsePathfinding(true);
        MoveRequest.SetAllowPartialPath(true);
        MoveRequest.SetCanStrafe(true);

        AIController->MoveTo(MoveRequest);

        // 핵심:
        // 너무 가까워도 일정 확률로 UsePhasePattern으로 넘긴다.
        // UsePhasePattern에서 가까운 상황이면 BasicArrow만 선택하게 만들 예정.
        if (bAllowBasicArrowWhileRetreating && FMath::FRand() <= RetreatBasicArrowChance)
        {
            return EBTNodeResult::Succeeded;
        }

        return EBTNodeResult::Failed;
    }

    // =========================
    // 2. 너무 멀면 접근한다.
    // =========================
    if (Distance > Data->DesiredCombatRange)
    {
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalActor(Target);
        MoveRequest.SetAcceptanceRadius(Data->DesiredCombatRange * 0.8f);
        MoveRequest.SetUsePathfinding(true);
        MoveRequest.SetAllowPartialPath(true);
        MoveRequest.SetCanStrafe(true);

        AIController->MoveTo(MoveRequest);

        return EBTNodeResult::Failed;
    }

    // =========================
    // 3. 적정 거리에서는 좌우 이동을 우선 시도한다.
    // =========================
    if (TryStartCombatStrafe(AIController, Boss, Target, Data))
    {
        return EBTNodeResult::Failed;
    }

    // 이동하지 않는 경우에만 공격 가능.
    return EBTNodeResult::Succeeded;
}

bool UBTTask_ShijuKeepRange::TryStartCombatStrafe(
    AAIController* AIController,
    AShijuBoss* Boss,
    AActor* Target,
    UShijuBossDataAsset* Data
)
{
    if (!bEnableCombatStrafe || !AIController || !Boss || !Target || !Data)
    {
        return false;
    }

    UWorld* World = Boss->GetWorld();
    if (!World)
    {
        return false;
    }

    UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent();
    if (PathFollowingComp && PathFollowingComp->GetStatus() == EPathFollowingStatus::Moving)
    {
        return true;
    }

    const float CurrentTime = World->GetTimeSeconds();

    if ((CurrentTime - LastCombatStrafeTime) < CombatStrafeCooldown)
    {
        return false;
    }

    if (FMath::FRand() > CombatStrafeChance)
    {
        return false;
    }

    if (FMath::FRand() < StrafeDirectionChangeChance)
    {
        StrafeDirectionSign *= -1;
    }

    FVector StrafeLocation = BuildCombatStrafeLocation(Boss, Target, Data);

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(StrafeLocation);
    MoveRequest.SetAcceptanceRadius(CombatStrafeAcceptanceRadius);
    MoveRequest.SetUsePathfinding(true);
    MoveRequest.SetAllowPartialPath(true);
    MoveRequest.SetCanStrafe(true);

    FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);

    if (MoveResult.Code == EPathFollowingRequestResult::Failed)
    {
        StrafeDirectionSign *= -1;
        StrafeLocation = BuildCombatStrafeLocation(Boss, Target, Data);

        MoveRequest.SetGoalLocation(StrafeLocation);
        MoveResult = AIController->MoveTo(MoveRequest);

        if (MoveResult.Code == EPathFollowingRequestResult::Failed)
        {
            return false;
        }
    }

    LastCombatStrafeTime = CurrentTime;

    UE_LOG(
        LogTemp,
        Verbose,
        TEXT("[Shiju KeepRange] Combat Strafe | Location = %s"),
        *StrafeLocation.ToString()
    );

    return true;
}

FVector UBTTask_ShijuKeepRange::BuildCombatStrafeLocation(
    AShijuBoss* Boss,
    AActor* Target,
    UShijuBossDataAsset* Data
) const
{
    const FVector BossLocation = Boss->GetActorLocation();
    const FVector TargetLocation = Target->GetActorLocation();

    FVector FromTargetToBoss = BossLocation - TargetLocation;
    FromTargetToBoss.Z = 0.f;
    FromTargetToBoss = FromTargetToBoss.GetSafeNormal();

    if (FromTargetToBoss.IsNearlyZero())
    {
        FromTargetToBoss = -Boss->GetActorForwardVector();
        FromTargetToBoss.Z = 0.f;
        FromTargetToBoss = FromTargetToBoss.GetSafeNormal();
    }

    const FVector TangentDirection(
        -FromTargetToBoss.Y * static_cast<float>(StrafeDirectionSign),
        FromTargetToBoss.X * static_cast<float>(StrafeDirectionSign),
        0.f
    );

    const FVector TangentMove = TangentDirection * CombatStrafeDistance;

    const FVector IdealRangeLocation =
        TargetLocation + FromTargetToBoss * Data->DesiredCombatRange;

    const FVector RadialCorrection =
        (IdealRangeLocation - BossLocation) * CombatStrafeRadialCorrectionWeight;

    FVector DesiredLocation = BossLocation + TangentMove + RadialCorrection;
    DesiredLocation.Z = BossLocation.Z;

    return DesiredLocation;
}

void UBTTask_ShijuKeepRange::DrawBossRangeDebug(
    AShijuBoss* Boss,
    UShijuBossDataAsset* Data
) const
{
    if (!bDrawBossRangeDebug || !Boss || !Data)
    {
        return;
    }

    UWorld* World = Boss->GetWorld();
    if (!World)
    {
        return;
    }

    const FVector Center = Boss->GetActorLocation() + FVector(0.f, 0.f, RangeDebugHeight);

    DrawDebugCircle(
        World,
        Center,
        Data->TooCloseRange,
        64,
        FColor::Red,
        false,
        0.f,
        0,
        2.f,
        FVector(1.f, 0.f, 0.f),
        FVector(0.f, 1.f, 0.f),
        false
    );

    DrawDebugCircle(
        World,
        Center,
        Data->DesiredCombatRange,
        96,
        FColor::Green,
        false,
        0.f,
        0,
        3.f,
        FVector(1.f, 0.f, 0.f),
        FVector(0.f, 1.f, 0.f),
        false
    );
}