#pragma once

#include "CoreMinimal.h"
#include "../BossBase.h"
#include "ShijuBoss.generated.h"

class UShijuBossDataAsset;
class AShijuArrowProjectile;
class AShijuQArea;
struct FBranchingPointNotifyPayload;

UCLASS()
class COF_CHARACTER_API AShijuBoss : public ABossBase
{
    GENERATED_BODY()

public:
    AShijuBoss();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Shiju|Target")
    void SetCurrentTarget(AActor* InTarget);

    UFUNCTION(BlueprintCallable, Category = "Shiju|Target")
    AActor* GetCurrentTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Shiju|Data")
    UShijuBossDataAsset* GetShijuData() const;

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    bool CanUseBasicArrow() const;

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    bool CanUsePiercingShot() const;

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    bool CanUseQ() const;

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    bool CanUseR() const;

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    bool PerformBasicArrowAttack();

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    bool PerformPiercingShot();

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    bool PerformQTimeRain();

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    bool PerformRSkill();

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    void FaceTarget();

    UFUNCTION(BlueprintPure, Category = "Shiju|Combat")
    bool IsAttackInProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Shiju|Combat")
    void FirePendingPiercingShot();

    UFUNCTION(BlueprintCallable, Category = "Shiju|Debug")
    void DebugApplyDamageToSelf(float DamageAmount);

    UFUNCTION(BlueprintPure, Category = "Shiju|Phase")
    int32 GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Shiju|Pattern")
    bool TryUsePhasePattern();

    UFUNCTION()
    void EndAttack();

    // 패시브 : 시간 표식
    void RegisterTimeMarkHit();


    UFUNCTION(BlueprintCallable, Category = "Shiju|Bell")
    void EmpowerNextArrow(float InDamageMultiplier, float InSpeedMultiplier);

protected:
    UFUNCTION()
    void HandleMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload);

    bool ResolveQGroundTargetLocation(FVector& OutGroundLocation) const;

    void QueueProjectileSpawn(TSubclassOf<AShijuArrowProjectile> InProjectileClass, float InDamage, float InSpeed, float Delay);
    void SpawnQueuedProjectile();

    AShijuArrowProjectile* SpawnProjectileTowardsCurrentTarget(
        TSubclassOf<AShijuArrowProjectile> InProjectileClass,
        float InDamage,
        float InSpeed
    );

    void SpawnQProjectileArc();

    void FireRArrowOnce();

    void ResetTimeMarkStack();
    void TriggerBellPassive();
    void EndBellPassiveCooldown();

    void UpdatePhaseByHP();
    void HandlePhaseChanged(int32 OldPhase, int32 NewPhase);
    void EnterPhase3();
    void RemovePhase3Pillars();

    int32 GetCurrentMaxTimeMarkStack() const;

    TSubclassOf<AShijuArrowProjectile> GetBasicProjectileClass() const;
    TSubclassOf<AShijuArrowProjectile> GetPiercingProjectileClass() const;
    TSubclassOf<AShijuArrowProjectile> GetQProjectileClass() const;
    TSubclassOf<AShijuArrowProjectile> GetRProjectileClass() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    TObjectPtr<AActor> CurrentTarget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shiju|Combat")
    FName ArrowSpawnSocketName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|Phase")
    int32 CurrentPhase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|Phase")
    bool bPhase3Entered;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    float LastBasicArrowTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    float LastPiercingShotTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    float LastQTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    float LastRTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    bool bAttackInProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    FVector CachedQTargetLocation;

    UPROPERTY()
    TSubclassOf<AShijuArrowProjectile> PendingProjectileClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    float PendingProjectileDamage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    float PendingProjectileSpeed;

    UPROPERTY()
    TObjectPtr<AShijuQArea> ActiveQWarningArea;

    // R 런타임
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|R")
    bool bRSkillActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|R")
    int32 RShotsFiredThisCast;

    // 패시브 런타임
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|Passive")
    int32 CurrentTimeMarkStack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|Passive")
    float SavedHPAtFirstMark;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|Passive")
    bool bBellPassiveCoolingDown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|State")
    bool bPiercingProjectilePending;
    void ApplyAndConsumeNextArrowEmpower(float& InOutDamage, float& InOutSpeed);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|Bell")
    bool bNextArrowEmpowered;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|Bell")
    float NextArrowDamageMultiplier;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shiju|Bell")
    float NextArrowSpeedMultiplier;

    FTimerHandle TimeMarkResetTimerHandle;
    FTimerHandle BellPassiveCooldownTimerHandle;
};