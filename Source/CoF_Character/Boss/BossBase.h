#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BossBase.generated.h"

class UBossDataAsset;
class UAnimMontage;

UCLASS()
class COF_CHARACTER_API ABossBase : public ACharacter
{
    GENERATED_BODY()

public:
    ABossBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        class AActor* DamageCauser
    ) override;

    UFUNCTION(BlueprintCallable, Category = "Boss")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Boss")
    void StartAttack();

    UFUNCTION(BlueprintCallable, Category = "Boss")
    void MeleeAttack();

    UFUNCTION(BlueprintCallable, Category = "Boss")
    void FinishAttack();

    UFUNCTION(BlueprintCallable, Category = "Boss")
    void Die();

    UFUNCTION(BlueprintPure, Category = "Boss")
    float GetHPPercent() const;

    UFUNCTION(BlueprintPure, Category = "Boss")
    bool IsDead() const { return bDead; }

    UFUNCTION(BlueprintPure, Category = "Boss")
    bool IsAttacking() const { return bAttacking; }

    UFUNCTION(BlueprintPure, Category = "Boss|Data")
    UBossDataAsset* GetBossData() const { return BossData; }

protected:
    UFUNCTION(BlueprintCallable, Category = "Boss")
    void ApplyBossData();

    UFUNCTION(BlueprintImplementableEvent, Category = "Boss")
    void OnBossDead();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Data")
    TObjectPtr<UBossDataAsset> BossData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    float CurrentHP;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    bool bDead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    bool bAttacking;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    float LastAttackTime;

private:
    FTimerHandle AttackFinishTimer;

    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};