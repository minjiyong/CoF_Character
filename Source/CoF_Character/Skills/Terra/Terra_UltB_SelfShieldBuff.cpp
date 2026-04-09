#include "Skills/Terra/Terra_UltB_SelfShieldBuff.h"

#include "TP_Character.h"
#include "HealthComponent.h"

UWorld* UTerra_UltB_SelfShieldBuff::GetWorld() const
{
	if (ATP_Character* C = GetOwnerChar())
		return C->GetWorld();
	return nullptr;
}

void UTerra_UltB_SelfShieldBuff::BuffStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	// 런타임 버프 상태는 스킬 객체가 들고 있음 (TP_Character 멤버 X)
	if (bActive) return;
	bActive = true;

	// 공격력 버프
	C->AttackMultiplier = C->UltB_AttackMultiplier;

	// 체력 버프: HealthComponent 에서 처리. 보호막 형태로 변경
	if (C->HealthComp)
	{
		C->HealthComp->AddShield(C->UltB_Shield);
	}

	// 버프 종료 타이머 - 시간이 지나면 BuffEnd가 호출되고 보호막이 제거됨.
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(EndTimerHandle);
		W->GetTimerManager().SetTimer(
			EndTimerHandle,
			this,
			&UTerra_UltB_SelfShieldBuff::BuffEnd,
			C->UltB_Duration,
			false
		);
	}

	// 버프는 시전 후에는 다시 입력 허용
	C->SetEveryInputEnabled(true);
}

void UTerra_UltB_SelfShieldBuff::BuffEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	// 런타임 버프 상태는 스킬 객체가 들고 있음 (TP_Character 멤버 X)
	if (!bActive) return;
	bActive = false;

	C->AttackMultiplier = 1.0f;

	if (C->HealthComp)
	{
		C->HealthComp->RemoveShield(C->UltB_Shield); // 보호막 제거
	}

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(EndTimerHandle);
	}

	// 버프 끝났을 때 입력 정책 - 버프 시전됐을 때 입력 허용했으니 여기는 없어도 될듯
}