#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BossAIController.generated.h"

class UBehaviorTree;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class COF_CHARACTER_API ABossAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABossAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION()
    void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|AI")
    TObjectPtr<UAIPerceptionComponent> AIPerception;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|AI")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|AI")
    TObjectPtr<UBehaviorTree> DefaultBehaviorTree;
};