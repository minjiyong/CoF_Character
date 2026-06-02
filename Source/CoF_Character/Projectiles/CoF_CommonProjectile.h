#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoF_CommonProjectile.generated.h"

class ATP_Character;
class UCombatComponent;
class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class COF_CHARACTER_API ACoF_CommonProjectile : public AActor
{
    GENERATED_BODY()

public:
    ACoF_CommonProjectile();

    void InitProjectile(
        ATP_Character* InOwnerCharacter,
        UCombatComponent* InCombatComp,
        UObject* InOwningSkill,
        float InDamage,
        float InInitialSpeed,
        float InLifeSeconds,
        float InRadius);

    // 포물선 발사용 - 기존 직선 발사와 분리
    void InitProjectileArc(
        ATP_Character* InOwnerCharacter,
        UCombatComponent* InCombatComp,
        UObject* InOwningSkill,
        float InDamage,
        const FVector& InLaunchVelocity,
        float InLifeSeconds,
        float InRadius,
        float InGravityScale = 1.0f);

protected:
    virtual void BeginPlay() override;
    virtual void LifeSpanExpired() override;

    UFUNCTION()
    void HandleOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleProjectileStop(const FHitResult& ImpactResult);

    void ResolveAndDestroy(const FVector& MarkLocation, const FVector& MarkNormal);

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> Collision = nullptr;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

    TWeakObjectPtr<ATP_Character> OwnerCharacter;
    TWeakObjectPtr<UCombatComponent> OwningCombatComp;
    TWeakObjectPtr<UObject> OwningSkill;

    float Damage = 0.f;
    bool bResolved = false;
};