#include "Skills/Terra/Terra_UltA_AllyShield.h"

#include "TP_Character.h"
#include "HealthComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

void UTerra_UltA_AllyShield::ShieldStart()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	if (!C->GetWorld()) return;

	// 이미 남아있는 기록이 있으면 정리(중복 시전 방지)
	C->UltA_ShieldGiven.Reset();

	const FVector Center = C->GetActorLocation();
	const float Radius = C->UltA_Radius;

	TArray<FOverlapResult> Hits;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UltA_Shield), false, C);

	const bool bAny = C->GetWorld()->OverlapMultiByChannel(
		Hits,
		Center,
		FQuat::Identity,
		ECC_Pawn, // 아군 더미가 Pawn이어야 함(아래 더미 제작 참고)
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	DrawDebugSphere(C->GetWorld(), Center, Radius, 24, bAny ? FColor::Red : FColor::Green, false, 1.0f, 0, 2.f);

	if (bAny)
	{
		for (const FOverlapResult& R : Hits)
		{
			AActor* Target = R.GetActor();
			if (!Target || Target == C) continue;

			// 아군 판별(태그)
			if (!Target->ActorHasTag(FName(TEXT("Ally"))))
				continue;

			UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>();
			if (!HC) continue;

			HC->AddShield(C->UltA_Shield);
			C->UltA_ShieldGiven.Add(Target, C->UltA_Shield);
		}
	}

	// 버프 종료 타이머 - 시간이 지나면 ShieldEnd가 호출되고 보호막이 제거됨.
	C->GetWorld()->GetTimerManager().ClearTimer(C->UltA_EndTimerHandle);
	C->GetWorld()->GetTimerManager().SetTimer(
		C->UltA_EndTimerHandle,
		C,
		&ATP_Character::UltA_ShieldEnd,
		C->UltA_Duration,
		false
	);

	// 버프는 시전 후에는 다시 입력 허용
	C->SetEveryInputEnabled(true);
}

void UTerra_UltA_AllyShield::ShieldEnd()
{
	ATP_Character* C = GetOwnerChar();
	if (!C) return;

	// 저장해둔 대상들에게서 “이번 UltA로 준 양만큼” 제거
	for (auto It = C->UltA_ShieldGiven.CreateIterator(); It; ++It)
	{
		AActor* Target = It.Key().Get();
		const float Given = It.Value();

		if (!Target) continue;

		UHealthComponent* HC = Target->FindComponentByClass<UHealthComponent>();
		if (!HC) continue;

		HC->RemoveShield(Given);
	}

	C->UltA_ShieldGiven.Reset();

	if (C->GetWorld())
		C->GetWorld()->GetTimerManager().ClearTimer(C->UltA_EndTimerHandle);
}