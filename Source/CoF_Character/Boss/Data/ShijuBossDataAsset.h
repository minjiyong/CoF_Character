#pragma once

#include "CoreMinimal.h"
#include "BossDataAsset.h"
#include "ShijuBossDataAsset.generated.h"

class UAnimMontage;
class USoundBase;
class AShijuArrowProjectile;
class AShijuQArea;

UCLASS(BlueprintType)
class COF_CHARACTER_API UShijuBossDataAsset : public UBossDataAsset
{
    GENERATED_BODY()

public:
    UShijuBossDataAsset();

public:
    // =========================
    // 기본 전투
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float DesiredCombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float TooCloseRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float BasicArrowCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float PiercingShotCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float PiercingChargeTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float BasicArrowDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float PiercingArrowDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float ArrowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Combat", meta = (ClampMin = "0.0"))
    float PiercingProjectileSpeed;

    // =========================
    // Q
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q", meta = (ClampMin = "0.0"))
    float QCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q", meta = (ClampMin = "0.0"))
    float QCastDelay;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q", meta = (ClampMin = "0.0"))
    float QRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q", meta = (ClampMin = "0.0"))
    float QDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q", meta = (ClampMin = "0.0"))
    float QMaxRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q", meta = (ClampMin = "0.0"))
    float QArcHeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q", meta = (ClampMin = "0.0"))
    float QBurnDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q", meta = (ClampMin = "0.05"))
    float QBurnTickInterval;

    // =========================
    // R : 8발 연사
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|R", meta = (ClampMin = "0.0"))
    float RCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|R", meta = (ClampMin = "1"))
    int32 RArrowCount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|R", meta = (ClampMin = "0.01"))
    float RFireInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|R", meta = (ClampMin = "0.0"))
    float RStartDelay;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|R", meta = (ClampMin = "0.0"))
    float RArrowDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|R", meta = (ClampMin = "0.0"))
    float RProjectileSpeed;

    // =========================
    // 패시브 : 시간 표식
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Passive", meta = (ClampMin = "0.1"))
    float TimeMarkDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Passive", meta = (ClampMin = "1"))
    int32 MaxTimeMarkStack;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Passive", meta = (ClampMin = "0.0"))
    float BellPassiveCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Passive")
    TObjectPtr<USoundBase> BellTriggerSound;

    // =========================
    // Phase
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Phase2StartHPRatio;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Phase3StartHPRatio;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Phase", meta = (ClampMin = "1"))
    int32 Phase3MaxTimeMarkStack;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Phase")
    FName Phase3RemovePillarTag;

    // =========================
    // Animation
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Animation")
    TObjectPtr<UAnimMontage> BasicArrowMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Animation")
    TObjectPtr<UAnimMontage> PiercingShotMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Animation")
    TObjectPtr<UAnimMontage> QMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Animation")
    TObjectPtr<UAnimMontage> RMontage;

    // =========================
    // Projectile
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Projectile")
    TSubclassOf<AShijuArrowProjectile> ArrowProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Projectile")
    TSubclassOf<AShijuArrowProjectile> BasicArrowProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Projectile")
    TSubclassOf<AShijuArrowProjectile> PiercingProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Projectile")
    TSubclassOf<AShijuArrowProjectile> QProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Projectile")
    TSubclassOf<AShijuArrowProjectile> RProjectileClass;

    // =========================
    // Actor Class
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Q|Actor")
    TSubclassOf<AShijuQArea> QAreaActorClass;
};