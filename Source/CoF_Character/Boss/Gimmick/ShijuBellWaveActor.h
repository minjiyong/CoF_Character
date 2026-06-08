#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShijuBellWaveActor.generated.h"

class USceneComponent;
class USphereComponent;
class UNiagaraComponent;
class ACharacter;
class AShijuBoss;

UCLASS()
class COF_CHARACTER_API AShijuBellWaveActor : public AActor
{
    GENERATED_BODY()

public:
    AShijuBellWaveActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Shiju|BellWave")
    void InitBellWave(
        AShijuBoss* InShijuBoss,
        float InMaxRadius,
        float InExpansionDuration,
        float InDamage,
        bool bInApplyTimeMark
    );

protected:
    void ScanCharactersInWave();
    void HandleWaveHit(ACharacter* Character);
    void EndBellWave();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BellWave")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BellWave")
    TObjectPtr<USphereComponent> WaveSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BellWave|FX")
    TObjectPtr<UNiagaraComponent> WaveFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BellWave", meta = (ClampMin = "1.0"))
    float MaxRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BellWave", meta = (ClampMin = "0.01"))
    float ExpansionDuration;

    // 충격파 판정 두께
    // 현재 반경을 중심으로 ±WaveThickness * 0.5 범위 안에 들어온 캐릭터만 맞는다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BellWave", meta = (ClampMin = "1.0"))
    float WaveThickness;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BellWave", meta = (ClampMin = "0.0"))
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BellWave")
    bool bApplyTimeMark;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BellWave")
    bool bAffectOnlyPlayerControlled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BellWave")
    bool bDrawDebugWave;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BellWave")
    float CurrentRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BellWave")
    float ElapsedTime;

    UPROPERTY()
    TObjectPtr<AShijuBoss> ShijuBoss;

    TSet<TWeakObjectPtr<ACharacter>> HitCharacters;
};