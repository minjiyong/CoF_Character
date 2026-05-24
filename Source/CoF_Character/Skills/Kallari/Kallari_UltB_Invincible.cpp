#include "Skills/Kallari/Kallari_UltB_Invincible.h"

#include "TP_Character.h"

UWorld* UKallari_UltB_Invincible::GetWorld() const
{
	if (ATP_Character* C = GetOwnerChar())
		return C->GetWorld();

	return nullptr;
}

void UKallari_UltB_Invincible::BuffStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	// 런타임 무적 상태는 스킬 객체가 들고 있음
	if (bActive) return;

	bActive = true;

	// 피격 무시
	C->bCanBeHit = false;

	// 함수 호출 - BP에 
	C->BP_UltBVisualStart();

	// 무적 종료 타이머
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(EndTimerHandle);
		W->GetTimerManager().SetTimer(
			EndTimerHandle,
			this,
			&UKallari_UltB_Invincible::BuffEnd,
			C->UltB_InvincibleDuration,
			false
		);
	}

	// 시전 후에는 다시 입력 허용
	C->SetEveryInputEnabled(true);
}

void UKallari_UltB_Invincible::BuffEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (!bActive) return;

	bActive = false;

	// 다시 피격 가능
	C->bCanBeHit = true;

	// 함수 호출 - BP에 
	C->BP_UltBVisualEnd();

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(EndTimerHandle);
	}
}