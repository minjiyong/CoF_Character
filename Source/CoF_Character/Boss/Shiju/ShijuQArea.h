#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShijuQArea.generated.h"

class USceneComponent;
class USphereComponent;
class UNiagaraComponent;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class COF_CHARACTER_API AShijuQArea : public AActor
{
    GENERATED_BODY()

public:
    AShijuQArea();

protected:
    virtual void BeginPlay() override;

public:
    void InitWarning(
        float InRadius,
        float InDamagePerTick,
        float InDuration,
        float InTickInterval,
        AActor* InDamageCauser
    );

    void ActivateBurnField(const FVector& ImpactLocation);

protected:
    void CacheEffectComponents();
    void ApplyBurnDamage();
    void EndBurnField();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QArea")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QArea")
    TObjectPtr<USphereComponent> AreaSphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QArea|FX")
    TObjectPtr<UParticleSystem> ImpactParticleAsset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QArea|FX")
    FVector ImpactParticleScale = FVector(1.f, 1.f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QArea|FX")
    FVector WarningFXScale = FVector(1.f, 1.f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QArea|FX")
    FVector MagicCircleScale = FVector(1.f, 1.f, 1.f);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QArea|Damage")
    float DamagePerTick;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QArea|Damage")
    float BurnDuration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QArea|Damage")
    float BurnTickInterval;

    UPROPERTY()
    TObjectPtr<AActor> DamageCauserActor;

    UPROPERTY(Transient)
    TObjectPtr<UNiagaraComponent> WarningFXComp;

    UPROPERTY(Transient)
    TObjectPtr<UParticleSystemComponent> MagicCircleComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QArea|State")
    bool bBurnActive;

    FTimerHandle BurnTickHandle;
    FTimerHandle BurnEndHandle;
};