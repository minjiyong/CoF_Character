#include "ShijuAnimInstance.h"
#include "../Shiju/ShijuBoss.h"

void UShijuAnimInstance::AnimNotify_P_Fire()
{
    AShijuBoss* Boss = Cast<AShijuBoss>(TryGetPawnOwner());
    if (!Boss)
    {
        return;
    }

    Boss->FirePendingPiercingShot();
}