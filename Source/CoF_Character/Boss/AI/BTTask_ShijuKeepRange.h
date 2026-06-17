#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ShijuKeepRange.generated.h"

class AAIController;
class AShijuBoss;
class UShijuBossDataAsset;

UCLASS()
class COF_CHARACTER_API UBTTask_ShijuKeepRange : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ShijuKeepRange();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Strafe")
    bool bEnableCombatStrafe;

    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Strafe", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CombatStrafeChance;

    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Strafe", meta = (ClampMin = "0.0"))
    float CombatStrafeCooldown;

    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Strafe", meta = (ClampMin = "0.0"))
    float CombatStrafeDistance;

    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Strafe", meta = (ClampMin = "0.0"))
    float CombatStrafeAcceptanceRadius;

    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Strafe", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CombatStrafeRadialCorrectionWeight;

    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Strafe", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StrafeDirectionChangeChance;

    // 너무 가까울 때 뒤로 빠지면서 BasicArrow를 허용할지
    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|RetreatShot")
    bool bAllowBasicArrowWhileRetreating;

    // 후퇴 중 BasicArrow를 항상 허용하면 너무 많이 쏠 수 있으므로 확률 적용
    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|RetreatShot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RetreatBasicArrowChance;

    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Debug")
    bool bDrawBossRangeDebug;

    UPROPERTY(EditAnywhere, Category = "Shiju|KeepRange|Debug", meta = (ClampMin = "0.0"))
    float RangeDebugHeight;

protected:
    bool TryStartCombatStrafe(
        AAIController* AIController,
        AShijuBoss* Boss,
        AActor* Target,
        UShijuBossDataAsset* Data
    );

    FVector BuildCombatStrafeLocation(
        AShijuBoss* Boss,
        AActor* Target,
        UShijuBossDataAsset* Data
    ) const;

    void DrawBossRangeDebug(
        AShijuBoss* Boss,
        UShijuBossDataAsset* Data
    ) const;

protected:
    float LastCombatStrafeTime;
    int32 StrafeDirectionSign;
};