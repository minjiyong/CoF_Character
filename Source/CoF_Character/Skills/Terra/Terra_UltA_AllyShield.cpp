#include "Skills/Terra/Terra_UltA_AllyShield.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "HealthComponent.h"
#include "TimerManager.h"
#include "TP_Character.h"

UWorld* UTerra_UltA_AllyShield::GetWorld() const
{
	if (ATP_Character* C = GetOwnerChar())
	{
		return C->GetWorld();
	}

	return nullptr;
}

void UTerra_UltA_AllyShield::ShieldStart()
{
	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return;
	}

	UWorld* W = C->GetWorld();

	if (!W)
	{
		return;
	}

	// 이전 실행에서 남은 타이머를 제거한다.
	W->GetTimerManager().ClearTimer(EndTimerHandle);

	// 이번 궁극기로 부여한 쉴드 목록을 초기화한다.
	ShieldGiven.Reset();

	// 이전에 남아 있던 쉴드 FX가 있다면 제거한다.
	ClearShieldFX();

	const FVector Center = C->GetActorLocation();
	const float Radius = C->UltA_Radius;

	TArray<FOverlapResult> Overlaps;

	// 시전자 본인은 Overlap 검색에서 제외한다.
	FCollisionQueryParams Params(
		SCENE_QUERY_STAT(TerraUltA_Shield),
		false,
		C
	);

	const bool bAnyOverlap = W->OverlapMultiByChannel(
		Overlaps,
		Center,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	if (bAnyOverlap)
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Target = Overlap.GetActor();

			if (!Target)
			{
				continue;
			}

			// 현재 UltA는 시전자 본인에게 적용하지 않는다.
			if (Target == C)
			{
				continue;
			}

			// Ally 태그가 있는 대상만 쉴드를 받는다.
			if (!Target->ActorHasTag(FName(TEXT("Ally"))))
			{
				continue;
			}

			UHealthComponent* HC =
				Target->FindComponentByClass<UHealthComponent>();

			if (!HC)
			{
				continue;
			}

			HC->AddShield(C->UltA_Shield);

			ShieldGiven.Add(
				Target,
				C->UltA_Shield
			);

			// 쉴드를 받은 대상의 FX_Center 소켓에 보호막 FX를 붙인다.
			SpawnShieldFX(Target);
		}
	}

	W->GetTimerManager().SetTimer(
		EndTimerHandle,
		this,
		&UTerra_UltA_AllyShield::ShieldEnd,
		C->UltA_Duration,
		false
	);
}

void UTerra_UltA_AllyShield::ShieldEnd()
{
	for (auto& ShieldPair : ShieldGiven)
	{
		AActor* Target = ShieldPair.Key.Get();
		const float GivenShield = ShieldPair.Value;

		if (!IsValid(Target))
		{
			continue;
		}

		UHealthComponent* HC =
			Target->FindComponentByClass<UHealthComponent>();

		if (!HC)
		{
			continue;
		}

		HC->RemoveShield(GivenShield);
	}

	ShieldGiven.Reset();

	// 쉴드 종료와 함께 대상에게 붙어 있던 FX도 제거한다.
	ClearShieldFX();

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(EndTimerHandle);
	}
}

void UTerra_UltA_AllyShield::SpawnShieldFX(AActor* Target)
{
	ATP_Character* C = GetOwnerChar();

	if (!C)
	{
		return;
	}

	if (!Target)
	{
		return;
	}

	// FX Class는 쉴드를 받는 대상이 아니라
	// 궁극기를 시전하는 Terra 캐릭터 BP에 지정되어 있어야 한다.
	if (!C->TerraUltA_ShieldedFXClass)
	{
		return;
	}

	UWorld* W = GetWorld();

	if (!W)
	{
		return;
	}

	const TWeakObjectPtr<AActor> TargetKey(Target);

	// 같은 대상에게 FX가 중복 생성되는 것을 방지한다.
	if (ShieldFXGiven.Contains(TargetKey))
	{
		return;
	}

	USkeletalMeshComponent* TargetMesh = nullptr;

	if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
	{
		TargetMesh = TargetCharacter->GetMesh();
	}
	else
	{
		TargetMesh =
			Target->FindComponentByClass<USkeletalMeshComponent>();
	}

	if (!TargetMesh)
	{
		return;
	}

	// socket name
	const FName SocketName(TEXT("Root"));
	const bool bHasShieldSocket =
		TargetMesh->DoesSocketExist(SocketName);

	// 소켓이 있으면 해당 소켓의 월드 위치에서 바로 생성한다.
	// 소켓이 없으면 대상 Mesh의 월드 위치를 사용한다.
	const FTransform SpawnTransform = bHasShieldSocket
		? TargetMesh->GetSocketTransform(SocketName, RTS_World)
		: TargetMesh->GetComponentTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = C;
	SpawnParams.Instigator = C;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* FXActor = W->SpawnActor<AActor>(
		C->TerraUltA_ShieldedFXClass,
		SpawnTransform.GetLocation(),
		SpawnTransform.Rotator(),
		SpawnParams
	);

	if (!FXActor)
	{
		return;
	}

	if (bHasShieldSocket)
	{
		const bool bAttached = FXActor->AttachToComponent(
			TargetMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			SocketName
		);

		if (!bAttached)
		{
			FXActor->Destroy();
			return;
		}
	}
	else
	{
		const bool bAttached = FXActor->AttachToComponent(
			TargetMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale
		);

		if (!bAttached)
		{
			FXActor->Destroy();
			return;
		}
	}

	FXActor->SetActorRelativeLocation(FVector::ZeroVector);
	FXActor->SetActorRelativeRotation(FRotator::ZeroRotator);

	ShieldFXGiven.Add(
		TargetKey,
		FXActor
	);
}

void UTerra_UltA_AllyShield::ClearShieldFX()
{
	for (auto& FXPair : ShieldFXGiven)
	{
		AActor* FXActor = FXPair.Value.Get();

		if (!IsValid(FXActor))
		{
			continue;
		}

		FXActor->Destroy();
	}

	ShieldFXGiven.Reset();
}