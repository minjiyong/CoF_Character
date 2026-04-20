#pragma once

#include "CoreMinimal.h"
#include "Skills/CoF_SkillBase.h"
#include "Kallari_Skill2A_ShurikenTeleport.generated.h"

class AKallari_Skill2A_ShurikenProjectile;

UCLASS()
class COF_CHARACTER_API UKallari_Skill2A_ShurikenTeleport : public UCoF_SkillBase
{
    GENERATED_BODY()

public:
    double NextAvailableTime = 0.0;

    bool bHasTeleportMark = false;
    FVector TeleportMarkLocation = FVector::ZeroVector;
    FVector TeleportMarkNormal = FVector::UpVector;

    TWeakObjectPtr<AKallari_Skill2A_ShurikenProjectile> ActiveProjectile;

    void ResetRuntime()
    {
        NextAvailableTime = 0.0;
        bHasTeleportMark = false;
        TeleportMarkLocation = FVector::ZeroVector;
        TeleportMarkNormal = FVector::UpVector;
        ActiveProjectile.Reset();
    }

    bool IsInCooldown(double Now) const { return Now < NextAvailableTime; }
    void StartCooldown(double Now, float CooldownSec) { NextAvailableTime = Now + CooldownSec; }

public:
    void ThrowProjectile();
    void OnProjectileResolved(const FVector& InMarkLocation, const FVector& InMarkNormal);
};