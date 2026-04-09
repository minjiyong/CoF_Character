#include "Skills/Terra/Terra_UltB_SelfShieldBuff.h"

#include "TP_Character.h"
#include "HealthComponent.h"

void UTerra_UltB_SelfShieldBuff::BuffStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (C->bUltBActive) return;
	C->bUltBActive = true;

	// 공격력 버프
	C->AttackMultiplier = C->UltB_AttackMultiplier;

	// 체력 버프: HealthComponent 에서 처리. 보호막 형태로 변경
	if (C->HealthComp)
	{
		C->HealthComp->AddShield(C->UltB_Shield);
	}

	// 버프 종료 타이머 - 시간이 지나면 BuffEnd가 호출되고 보호막이 제거됨.
	C->GetWorld()->GetTimerManager().ClearTimer(C->UltB_EndTimerHandle);
	C->GetWorld()->GetTimerManager().SetTimer(
		C->UltB_EndTimerHandle,
		C,
		&ATP_Character::UltB_BuffEnd,
		C->UltB_Duration,
		false
	);

	// 버프는 시전 후에는 다시 입력 허용
	C->SetEveryInputEnabled(true);
}

void UTerra_UltB_SelfShieldBuff::BuffEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (!C->bUltBActive) return;
	C->bUltBActive = false;

	C->AttackMultiplier = 1.0f;

	if (C->HealthComp)
	{
		C->HealthComp->RemoveShield(C->UltB_Shield); // 보호막 제거
	}

	C->GetWorld()->GetTimerManager().ClearTimer(C->UltB_EndTimerHandle);

	// 버프 끝났을 때 입력 정책 - 버프 시전됐을 때 입력 허용했으니 여기는 없어도 될듯
}