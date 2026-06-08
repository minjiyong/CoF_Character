#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShijuArenaManager.generated.h"

class AShijuBoss;
class AShijuTimeRiftZone;
class AShijuBellWaveActor;
class ACharacter;

UCLASS()
class COF_CHARACTER_API AShijuArenaManager : public AActor
{
    GENERATED_BODY()

public:
    AShijuArenaManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

protected:
    void FindShijuBossIfNeeded();

    void UpdateTimeRiftSpawn();
    void SpawnTimeRift();

    ACharacter* ChoosePlayerForTimeRift() const;
    bool ResolveRandomGroundLocation(FVector& OutLocation) const;
    bool ResolveRandomGroundLocationNearPlayer(ACharacter* TargetPlayer, FVector& OutLocation) const;
    bool ResolveRandomGroundLocationInArena(FVector& OutLocation) const;

    void CleanupInvalidRifts();

    void UpdateBellWaveSpawn();
    void SpawnBellWave();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena")
    TObjectPtr<AShijuBoss> ShijuBoss;

    // =========================
    // Time Rift
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift")
    TSubclassOf<AShijuTimeRiftZone> TimeRiftZoneClass;

    // 기존 맵 중앙 기준 생성 반경.
    // bSpawnTimeRiftAroundPlayer가 false이거나 플레이어를 못 찾았을 때 fallback으로 사용한다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.0"))
    float SpawnRadius;

    // true면 TimeRift를 ArenaManager 주변이 아니라 플레이어 주변에 생성한다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift")
    bool bSpawnTimeRiftAroundPlayer;

    // 플레이어 발밑 즉시 생성 방지용 최소 거리.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.0"))
    float TimeRiftMinDistanceFromPlayer;

    // 플레이어 주변 생성 최대 거리.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.0"))
    float TimeRiftMaxDistanceFromPlayer;

    // 플레이어를 못 찾았을 때 기존 ArenaManager 중심 생성으로 대체할지 여부.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift")
    bool bFallbackToArenaCenterIfNoPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.0"))
    float GroundTraceHeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.0"))
    float GroundTraceDepth;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "1"))
    int32 MaxActiveRifts;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.1"))
    float Phase2SpawnInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.1"))
    float Phase3SpawnInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "1.0"))
    float TimeRiftRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.0"))
    float TimeRiftDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|TimeRift", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float TimeRiftSlowMultiplier;

    // =========================
    // Bell Wave
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|BellWave")
    bool bEnableBellWave;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|BellWave")
    TSubclassOf<AShijuBellWaveActor> BellWaveActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|BellWave", meta = (ClampMin = "0.1"))
    float Phase3BellWaveInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|BellWave", meta = (ClampMin = "1.0"))
    float BellWaveMaxRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|BellWave", meta = (ClampMin = "0.01"))
    float BellWaveExpansionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|BellWave", meta = (ClampMin = "0.0"))
    float BellWaveDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|BellWave")
    bool bBellWaveAppliesTimeMark;

    // =========================
    // Debug
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|Debug")
    bool bDebugLog;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|Arena|Debug")
    bool bDrawSpawnArea;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AShijuTimeRiftZone>> ActiveRifts;

    TWeakObjectPtr<AShijuBellWaveActor> ActiveBellWave;

    float LastTimeRiftSpawnTime;
    float LastBellWaveSpawnTime;
};