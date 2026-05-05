#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Kallari_Skill2B_ShurikenExplosion.generated.h"

class AKallari_Skill2A_ShurikenProjectile;

UCLASS()
class COF_CHARACTER_API UKallari_Skill2B_ShurikenExplosion : public UCoF_SkillBase
{
    GENERATED_BODY()

public:
    double NextAvailableTime = 0.0;

    bool bHasExplosionMark = false;
    FVector ExplosionMarkLocation = FVector::ZeroVector;
    FVector ExplosionMarkNormal = FVector::UpVector;

    TWeakObjectPtr<AKallari_Skill2A_ShurikenProjectile> ActiveProjectile;

    void ResetRuntime()
    {
        NextAvailableTime = 0.0;
        bHasExplosionMark = false;
        ExplosionMarkLocation = FVector::ZeroVector;
        ExplosionMarkNormal = FVector::UpVector;
        ActiveProjectile.Reset();
    }

    bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
    void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

    bool HasExplosionMark() const { return bHasExplosionMark; }

    void ThrowProjectile();
    void OnProjectileResolved(const FVector& InMarkLocation, const FVector& InMarkNormal);
    bool PlayExplosionMontage();
    void ExplodeAtMark();
};