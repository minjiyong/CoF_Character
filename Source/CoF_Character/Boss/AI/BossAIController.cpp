#include "BossAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../BossBase.h"
#include "../Data/BossDataAsset.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Pawn.h"

ABossAIController::ABossAIController()
{
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    SightConfig->SightRadius = 2000.f;
    SightConfig->LoseSightRadius = 2200.f;
    SightConfig->PeripheralVisionAngleDegrees = 70.f;
    SightConfig->SetMaxAge(2.0f);

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ABossAIController::HandleTargetPerceptionUpdated);
}

void ABossAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (ABossBase* Boss = Cast<ABossBase>(InPawn))
    {
        if (UBossDataAsset* BossData = Boss->GetBossData())
        {
            SightConfig->SightRadius = BossData->DetectRange;
            SightConfig->LoseSightRadius = BossData->DetectRange + 200.f;
            AIPerception->RequestStimuliListenerUpdate();
        }
    }

    if (!DefaultBehaviorTree || !InPawn)
    {
        return;
    }

    RunBehaviorTree(DefaultBehaviorTree);

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    BB->SetValueAsVector(TEXT("HomeLocation"), InPawn->GetActorLocation());
    BB->ClearValue(TEXT("TargetActor"));
    BB->SetValueAsBool(TEXT("bHasTarget"), false);
}

void ABossAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    UE_LOG(LogTemp, Warning,
        TEXT("[Perception] Actor=%s Sensed=%s"),
        *GetNameSafe(Actor),
        Stimulus.WasSuccessfullySensed() ? TEXT("true") : TEXT("false"));

    if (!Actor)
    {
        return;
    }

    APawn* SeenPawn = Cast<APawn>(Actor);
    if (!SeenPawn || !SeenPawn->IsPlayerControlled())
    {
        return;
    }

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        UE_LOG(LogTemp, Error, TEXT("[Perception] Blackboard is null"));
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        BB->SetValueAsObject(TEXT("TargetActor"), Actor);
        BB->SetValueAsBool(TEXT("bHasTarget"), true);
        UE_LOG(LogTemp, Warning, TEXT("[Perception] TargetActor set: %s"), *GetNameSafe(Actor));
    }
    else
    {
        UObject* CurrentTarget = BB->GetValueAsObject(TEXT("TargetActor"));
        if (CurrentTarget == Actor)
        {
            BB->ClearValue(TEXT("TargetActor"));
            BB->SetValueAsBool(TEXT("bHasTarget"), false);
            UE_LOG(LogTemp, Warning, TEXT("[Perception] TargetActor cleared"));
        }
    }
}