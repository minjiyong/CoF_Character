#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kallari_Skill2A_ShurikenProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UCombatComponent;
class ATP_Character;
class UKallari_Skill2A_ShurikenTeleport;

UCLASS()
class COF_CHARACTER_API AKallari_Skill2A_ShurikenProjectile : public AActor
{
    GENERATED_BODY()

public:
    AKallari_Skill2A_ShurikenProjectile();

    // Skill2_A / Skill2_B 공용 투사체 초기화
    void InitProjectile(
        ATP_Character* InOwnerCharacter,
        UCombatComponent* InCombatComp,
        UObject* InOwningSkill,
        float InDamage,
        float InInitialSpeed,
        float InLifeSeconds,
        float InRadius
    );

protected:
    virtual void BeginPlay() override;
    virtual void LifeSpanExpired() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> Collision = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

    UFUNCTION()
    void HandleOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void HandleProjectileStop(const FHitResult& ImpactResult);

private:
    void ResolveAndDestroy(const FVector& MarkLocation, const FVector& MarkNormal);

private:
    TWeakObjectPtr<ATP_Character> OwnerCharacter;
    TWeakObjectPtr<UCombatComponent> OwningCombatComp;
    TWeakObjectPtr<UObject> OwningSkill;

    float Damage = 0.f;
    bool bResolved = false;
};