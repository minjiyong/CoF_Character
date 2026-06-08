#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShijuBellShotManager.generated.h"

class AShijuBoss;
class AShijuArrowProjectile;
class AShijuBellTarget;

UCLASS()
class COF_CHARACTER_API AShijuBellShotManager : public AActor
{
    GENERATED_BODY()

public:
    AShijuBellShotManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

protected:
    void FindShijuBossIfNeeded();
    void UpdateBellShot();
    void FireBellShot();

    AShijuBellTarget* ChooseRandomBellTarget() const;

    bool ResolveBellShotSpawnTransform(
        AShijuBellTarget* BellTarget,
        FVector& OutSpawnLocation,
        FRotator& OutSpawnRotation
    ) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot")
    TObjectPtr<AShijuBoss> ShijuBoss;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot")
    TSubclassOf<AShijuArrowProjectile> BellShotProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot")
    FName ArrowSpawnSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot")
    bool bAutoFire;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot", meta = (ClampMin = "0.0"))
    float InitialDelay;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot", meta = (ClampMin = "0.1"))
    float Phase2FireInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot", meta = (ClampMin = "0.1"))
    float Phase3FireInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot", meta = (ClampMin = "0.0"))
    float ProjectileSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot", meta = (ClampMin = "0.0"))
    float ProjectileDamage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot")
    bool bRotateBossTowardBell;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shiju|BellShot")
    bool bDebugLog;

    float LastBellShotTime;
};