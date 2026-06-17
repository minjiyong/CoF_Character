#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShijuBellTarget.generated.h"

class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class ACharacter;
class AShijuBoss;
class AShijuTimeRiftZone;

UENUM(BlueprintType)
enum class EShijuBellEffectType : uint8
{
    Slow UMETA(DisplayName = "Slow"),
    TimeMark UMETA(DisplayName = "TimeMark"),
    Knockback UMETA(DisplayName = "Knockback"),
    PlayerNearbyTimeRift UMETA(DisplayName = "Player Nearby TimeRift"),
    EmpowerNextBossArrow UMETA(DisplayName = "Empower Next Boss Arrow"),
    PositionRewind UMETA(DisplayName = "Position Rewind")
};

struct FShijuBellSlowState
{
    float OriginalMaxWalkSpeed = 0.f;
    int32 ActiveSlowCount = 0;
};

struct FShijuPlayerPositionSample
{
    float TimeSeconds = 0.f;
    FVector Location = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
};

struct FShijuPlayerPositionHistory
{
    TArray<FShijuPlayerPositionSample> Samples;
    float LastSampleTime = -1000.f;
};

UCLASS()
class COF_CHARACTER_API AShijuBellTarget : public AActor
{
    GENERATED_BODY()

public:
    AShijuBellTarget();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

public:
    virtual float TakeDamage(
        float DamageAmount,
        FDamageEvent const& DamageEvent,
        AController* EventInstigator,
        AActor* DamageCauser
    ) override;

    UFUNCTION(BlueprintPure, Category = "Shiju|Bell")
    FVector GetBellAimLocation() const;

    UFUNCTION(BlueprintPure, Category = "Shiju|Bell")
    bool CanBeShot() const;

protected:
    void TriggerRandomBellEffect(AActor* DamageCauser);
    EShijuBellEffectType ChooseEffectByPhase(int32 CurrentPhase) const;

    void GetAffectedPlayerCharacters(TArray<ACharacter*>& OutCharacters) const;

    void ApplySlowEffect(const TArray<ACharacter*>& Characters);
    void ApplyTimeMarkEffect(const TArray<ACharacter*>& Characters, AActor* DamageCauser);
    void ApplyKnockbackEffect(const TArray<ACharacter*>& Characters);
    void ApplyPlayerNearbyTimeRiftEffect(const TArray<ACharacter*>& Characters);
    void ApplyEmpowerNextBossArrowEffect(AActor* DamageCauser);
    void ApplyPositionRewindEffect(const TArray<ACharacter*>& Characters);

    void ApplySlowToCharacter(ACharacter* Character);
    void RestoreSlowFromCharacter(ACharacter* Character);

    bool SpawnTimeRiftNearCharacter(ACharacter* Character);
    bool ResolveRandomGroundLocationNearCharacter(ACharacter* Character, FVector& OutLocation) const;

    void RecordPlayerPositionHistory();
    bool FindRewindTransform(ACharacter* Character, FVector& OutLocation, FRotator& OutRotation) const;

    void EndCooldown();

    FString GetEffectName(EShijuBellEffectType EffectType) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bell")
    TObjectPtr<USceneComponent> Root;

    // 화살 피격용 충돌. 크게 잡아도 되고 Pawn은 막지 않는다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bell")
    TObjectPtr<USphereComponent> BellCollision;

    // 플레이어 통과 방지용 충돌. 실제 종 크기에 가깝게 작게 잡는다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bell")
    TObjectPtr<USphereComponent> PlayerBlockCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bell")
    TObjectPtr<UStaticMeshComponent> BellMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Target")
    FVector AimOffset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Trigger")
    bool bOnlyTriggeredByShiju;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Trigger", meta = (ClampMin = "0.0"))
    float TriggerCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bell|Trigger")
    bool bCoolingDown;

    // 0이면 모든 플레이어에게 적용.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Effect", meta = (ClampMin = "0.0"))
    float EffectRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Effect")
    TArray<EShijuBellEffectType> Phase2EffectCandidates;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Effect")
    TArray<EShijuBellEffectType> Phase3EffectCandidates;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Slow", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float SlowMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Slow", meta = (ClampMin = "0.0"))
    float SlowDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Knockback", meta = (ClampMin = "0.0"))
    float KnockbackStrength;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Knockback", meta = (ClampMin = "0.0"))
    float KnockbackUpwardStrength;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|TimeRift")
    TSubclassOf<AShijuTimeRiftZone> TimeRiftZoneClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|TimeRift", meta = (ClampMin = "0.0"))
    float TimeRiftMinDistanceFromPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|TimeRift", meta = (ClampMin = "0.0"))
    float TimeRiftMaxDistanceFromPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|TimeRift", meta = (ClampMin = "1.0"))
    float TimeRiftRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|TimeRift", meta = (ClampMin = "0.0"))
    float TimeRiftDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|TimeRift", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float TimeRiftSlowMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|TimeRift", meta = (ClampMin = "0.0"))
    float GroundTraceHeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|TimeRift", meta = (ClampMin = "0.0"))
    float GroundTraceDepth;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Empower", meta = (ClampMin = "1.0"))
    float NextArrowDamageMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Empower", meta = (ClampMin = "1.0"))
    float NextArrowSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Rewind", meta = (ClampMin = "0.1"))
    float RewindSecondsBefore;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Rewind", meta = (ClampMin = "0.1"))
    float PositionHistoryMaxDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Rewind", meta = (ClampMin = "0.02"))
    float PositionHistorySampleInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Debug")
    bool bDebugLog;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bell|Debug")
    bool bDrawEffectRadius;

    static TMap<TWeakObjectPtr<ACharacter>, FShijuBellSlowState> GlobalSlowStates;

    TMap<TWeakObjectPtr<ACharacter>, FShijuPlayerPositionHistory> PlayerPositionHistories;

    FTimerHandle CooldownTimerHandle;
};