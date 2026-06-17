#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShijuArrowProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class AShijuQArea;

UCLASS()
class COF_CHARACTER_API AShijuArrowProjectile : public AActor
{
    GENERATED_BODY()

public:
    AShijuArrowProjectile();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnProjectileHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit
    );

public:
    void InitProjectile(float InDamage, AActor* InDamageCauser, float InSpeed);
    void InitQProjectile(AShijuQArea* InLinkedQArea);

    FORCEINLINE UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovement; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<USphereComponent> CollisionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UStaticMeshComponent> MeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    float Damage;

    UPROPERTY()
    TObjectPtr<AActor> DamageCauserActor;

    UPROPERTY()
    TObjectPtr<AShijuQArea> LinkedQArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    bool bIsQProjectile;
};