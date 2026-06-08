#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShijuTimeRiftZone.generated.h"

class USceneComponent;
class USphereComponent;
class UNiagaraComponent;
class ACharacter;

struct FShijuTimeRiftGlobalAffectedState
{
    float OriginalMaxWalkSpeed = 0.f;
    int32 ActiveRiftCount = 0;
};

UCLASS()
class COF_CHARACTER_API AShijuTimeRiftZone : public AActor
{
    GENERATED_BODY()

public:
    AShijuTimeRiftZone();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Shiju|TimeRift")
    void InitTimeRift(float InRadius, float InDuration, float InSlowMultiplier);

protected:
    UFUNCTION()
    void OnRiftBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnRiftEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );

    void RefreshAffectedCharacters();
    void ApplySlow(ACharacter* Character);
    void RestoreSlow(ACharacter* Character);
    void RestoreAllLocallyAffectedCharacters();
    void EndTimeRift();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeRift")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeRift")
    TObjectPtr<USphereComponent> RiftSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeRift|FX")
    TObjectPtr<UNiagaraComponent> RiftFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeRift", meta = (ClampMin = "0.0"))
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeRift", meta = (ClampMin = "0.0"))
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeRift", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float SlowMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeRift")
    bool bAutoDestroyAfterDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeRift")
    bool bAffectOnlyPlayerControlled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeRift|Debug")
    bool bDrawDebugSphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TimeRift|Debug", meta = (ClampMin = "0.02"))
    float ScanInterval;

    TSet<TWeakObjectPtr<ACharacter>> LocallyAffectedCharacters;

    static TMap<TWeakObjectPtr<ACharacter>, FShijuTimeRiftGlobalAffectedState> GlobalAffectedCharacters;

    FTimerHandle ScanTimerHandle;
    FTimerHandle EndTimerHandle;
};