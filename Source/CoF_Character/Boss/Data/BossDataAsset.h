#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BossDataAsset.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class COF_CHARACTER_API UBossDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UBossDataAsset();

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Info")
    FName BossName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Stat", meta = (ClampMin = "1.0"))
    float MaxHP;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Stat", meta = (ClampMin = "0.0"))
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Combat", meta = (ClampMin = "0.0"))
    float DetectRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Combat", meta = (ClampMin = "0.0"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Combat", meta = (ClampMin = "0.0"))
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Combat", meta = (ClampMin = "0.0"))
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Animation")
    TObjectPtr<UAnimMontage> IntroMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Animation")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Animation")
    TObjectPtr<UAnimMontage> HitReactMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Animation")
    TObjectPtr<UAnimMontage> DeadMontage;
};