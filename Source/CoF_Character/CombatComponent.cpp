#include "CombatComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionShape.h"

#include "HitReactInterface.h"
#include "Interfaces/DebuffBallTargetInterface.h"

#include "TP_Character.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bHitWindowOpen) return;
	if (HitQueryType != EHitQueryType::DashTrace) return;

	ProcessHitQuery(); // Dash인 경우만 반복할 수 있도록
}


void UCombatComponent::ConfigureTraceHit(float InDamage, float InRange, FName InStartSocket)
{
	HitQueryType = EHitQueryType::TraceForward;
	PendingDamage = InDamage;
	PendingRange = InRange;
	PendingTraceStartSocket = InStartSocket;
}

void UCombatComponent::ConfigureAOEHit(float InDamage, float InRadius)
{
	HitQueryType = EHitQueryType::AOESphere;
	PendingDamage = InDamage;
	PendingRadius = InRadius;
}

void UCombatComponent::ConfigureDashHit(float InDamage, float InDuration, float InRadius)
{
	HitQueryType = EHitQueryType::DashTrace;
	PendingDamage = InDamage;
	DashRadius = InRadius;

	if (UWorld* W = GetWorld())
	{
		DashEndTime = W->GetTimeSeconds() + InDuration;
	}
	else
	{
		DashEndTime = 0.0;
	}

	DashHitActors.Reset();

	if (AActor* Owner = GetOwner())
	{
		DashPrevLoc = Owner->GetActorLocation();
	}
}

void UCombatComponent::ConfigureAOEForwardHit(float InDamage, float InRadius, float InForwardOffset, float InHalfAngleDeg)
{
	HitQueryType = EHitQueryType::AOEForward;

	PendingDamage = InDamage;
	PendingRadius = InRadius;
	PendingForwardOffset = InForwardOffset;
	PendingHalfAngleDeg = InHalfAngleDeg;
}

void UCombatComponent::ConfigureAOELocationHit(const FVector& InCenter, float InDamage, float InRadius)
{
	HitQueryType = EHitQueryType::AOELocation;
	PendingAOELocation = InCenter;
	PendingDamage = InDamage;
	PendingRadius = InRadius;
}

void UCombatComponent::ConfigureSpinHit(float InDamagePerTick, float InRadius, float InTickInterval, float InDuration)
{
	HitQueryType = EHitQueryType::SpinSweep;

	SpinDamagePerTick = InDamagePerTick;
	SpinRadius = InRadius;
	SpinTickInterval = FMath::Max(0.01f, InTickInterval);

	UWorld* World = GetWorld();
	SpinEndTime = 0.0;
	if (World && InDuration > 0.f)
		SpinEndTime = World->GetTimeSeconds() + InDuration;

	SpinLastHitTime.Reset();

	if (AActor* Owner = GetOwner())
		SpinPrevLoc = Owner->GetActorLocation();
}

void UCombatComponent::ConfigureBeamSweepHit(float InDamage, float InRange, float InRadius, FName InStartSocket)
{
	HitQueryType = EHitQueryType::BeamSweepForward;
	PendingDamage = InDamage;
	PendingRange = InRange;
	PendingRadius = InRadius;
	PendingTraceStartSocket = InStartSocket;
}


void UCombatComponent::BeginHitWindow_OneShot()
{
	bHitWindowOpen = true;
	bHitAppliedThisSwing = false;

	ProcessHitQuery();
}

void UCombatComponent::EndHitWindow()
{
	bHitWindowOpen = false;
	bHitAppliedThisSwing = false;

	DashHitActors.Reset();     // 기존 dash
	SpinLastHitTime.Reset();   // spin
	SpinPrevLoc = FVector::ZeroVector;
	PendingAOELocation = FVector::ZeroVector;
	PendingTraceStartSocket = NAME_None;
}

void UCombatComponent::ProcessHitQuery()
{
	if (!bHitWindowOpen) return;
	if (bHitAppliedThisSwing) return;

	AActor* Owner = GetOwner();
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	if (!World) return;

	if (HitQueryType == EHitQueryType::TraceForward)
	{
		TArray<FHitResult> Hits;
		DoLineTraceMultiWithRange(Hits, PendingRange);
		ApplyHitResults(Hits, PendingDamage);
	}

	else if (HitQueryType == EHitQueryType::BeamSweepForward)
	{
		TArray<FHitResult> Hits;
		DoSphereSweepMultiWithRange(Hits, PendingRange, PendingRadius, PendingTraceStartSocket);
		ApplyHitResults(Hits, PendingDamage);
	}

	else if (HitQueryType == EHitQueryType::AOESphere)
	{
		const FVector Center = Owner->GetActorLocation();
		const float Radius = PendingRadius;

		TArray<FOverlapResult> Hits;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(SkillAOEHit), false, Owner);

		// Pawn만 맞추고 싶으면 ECC_Pawn / 더미가 다른 채널이면 바꿔야 함
		static_assert(sizeof(FOverlapResult) > 0, "FOverlapResult not defined here");
		const bool bAny = World->OverlapMultiByChannel(
			Hits,
			Center,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(Radius),
			Params
		);

		if (bDrawDebug)
		{
			DrawDebugSphere(World, Center, Radius, 24, bAny ? FColor::Red : FColor::Green, false, 1.0f, 0, 2.f);
		}

		if (bAny)
		{
			for (const FOverlapResult& R : Hits)
			{
				AActor* Target = R.GetActor();
				if (!Target || Target == Owner) continue;

				// AOE는 노멀을 임의로 UpVector로
				ApplyHitToActor(Target, PendingDamage, Center, FVector::UpVector);
			}
		}
	}

	else if (HitQueryType == EHitQueryType::DashTrace)
	{
		// 기간 종료
		if (DashEndTime > 0.0 && World->GetTimeSeconds() >= DashEndTime)
		{
			EndHitWindow();
			return;
		}

		const FVector CurrLoc = Owner->GetActorLocation();

		// 첫 프레임에 PrevLoc가 0이면 방어
		if (DashPrevLoc.IsZero())
		{
			DashPrevLoc = CurrLoc;
			return;
		}

		// Sweep: PrevLoc -> CurrLoc 사이를 “몸통 크기 구”로 쓸기
		TArray<FHitResult> Hits;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(DashSweep), false, Owner);

		const bool bAny = World->SweepMultiByChannel(
			Hits,
			DashPrevLoc,
			CurrLoc,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(DashRadius),
			Params
		);

		if (bDrawDebug)
		{
			// 이동 경로 표시(선) + 현재 위치 구
			DrawDebugLine(World, DashPrevLoc, CurrLoc, bAny ? FColor::Red : FColor::Green, false, 0.1f, 0, 2.f);
			DrawDebugSphere(World, CurrLoc, DashRadius, 16, FColor::Yellow, false, 0.1f, 0, 1.f);
		}

		if (bAny)
		{
			for (const FHitResult& H : Hits)
			{
				AActor* Target = H.GetActor();
				if (!Target || Target == Owner) continue;

				// 동일 대상은 돌진 중 1회만
				if (DashHitActors.Contains(Target)) continue;
				DashHitActors.Add(Target);

				ApplyHitToActor(Target, PendingDamage, H.ImpactPoint, H.ImpactNormal);
			}
		}

		// 다음 프레임을 위해 갱신
		DashPrevLoc = CurrLoc;
		return;
	}

	else if (HitQueryType == EHitQueryType::AOEForward)
	{
		if (bHitAppliedThisSwing) return;

		const FVector OwnerLoc = Owner->GetActorLocation();
		const FVector Fwd = Owner->GetActorForwardVector();

		// 전방 오프셋된 중심점 (방패가 닿는 앞쪽)
		const FVector Center = OwnerLoc + Fwd * PendingForwardOffset;

		const float Radius = PendingRadius;
		const float HalfAngleRad = FMath::DegreesToRadians(PendingHalfAngleDeg);
		const float CosThreshold = FMath::Cos(HalfAngleRad);

		TArray<FOverlapResult> Hits;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ForwardAOEHit), false, Owner);

		// 후보군을 구로 먼저 모으기 (타겟 타입에 맞는 채널 필요)
		const bool bAny = World->OverlapMultiByChannel(
			Hits,
			Center,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(Radius),
			Params
		);

		if (bDrawDebug)
		{
			DrawDebugSphere(World, Center, Radius, 24, bAny ? FColor::Red : FColor::Green, false, 1.0f, 0, 2.f);
			DrawDebugLine(World, OwnerLoc, Center, FColor::Cyan, false, 1.0f, 0, 2.f);

			const int32 Segs = 16; // 더 높이면 더 부드러움
			const float AngleStep = (PendingHalfAngleDeg * 2.f) / Segs;

			// 호(arc) 시작점
			FVector Prev = Center + Fwd.RotateAngleAxis(-PendingHalfAngleDeg, FVector::UpVector) * Radius;

			// 부채꼴의 호를 그림
			for (int32 i = 1; i <= Segs; ++i)
			{
				const float A = -PendingHalfAngleDeg + AngleStep * i;
				const FVector Dir = Fwd.RotateAngleAxis(A, FVector::UpVector);
				const FVector Curr = Center + Dir * Radius;

				DrawDebugLine(World, Prev, Curr, FColor::Cyan, false, 1.0f, 0, 1.5f);
				Prev = Curr;
			}

			// 방사선(왼/오 경계 + 중앙)
			const FVector DirL = Fwd.RotateAngleAxis(-PendingHalfAngleDeg, FVector::UpVector);
			const FVector DirR = Fwd.RotateAngleAxis(PendingHalfAngleDeg, FVector::UpVector);
			DrawDebugLine(World, Center, Center + DirL * Radius, FColor::Cyan, false, 1.0f, 0, 1.5f);
			DrawDebugLine(World, Center, Center + DirR * Radius, FColor::Cyan, false, 1.0f, 0, 1.5f);
			DrawDebugLine(World, Center, Center + Fwd * Radius, FColor::Blue, false, 1.0f, 0, 1.5f);
		}

		if (bAny)
		{
			for (const FOverlapResult& R : Hits)
			{
				AActor* Target = R.GetActor();
				if (!Target || Target == Owner) continue;

				// 전방 각도 필터 (부채꼴)
				const FVector ToTarget = (Target->GetActorLocation() - OwnerLoc).GetSafeNormal();
				const float Dot = FVector::DotProduct(Fwd, ToTarget);

				if (Dot < CosThreshold)
					continue;

				ApplyHitToActor(Target, PendingDamage, Center, FVector::UpVector);
			}
		}

		bHitAppliedThisSwing = true;
		return;
	}

	else if (HitQueryType == EHitQueryType::AOELocation)
	{
		AActor* OwnerActor = GetOwner();
		if (!OwnerActor || !World) return;

		TArray<FOverlapResult> Overlaps;
		FCollisionShape Shape = FCollisionShape::MakeSphere(PendingRadius);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(AOELocationHit), false, OwnerActor);
		Params.AddIgnoredActor(OwnerActor);

		const bool bHit = World->OverlapMultiByChannel(
			Overlaps,
			PendingAOELocation,
			FQuat::Identity,
			ECC_Pawn,
			Shape,
			Params
		);

		if (!bHit) return;

		TSet<AActor*> HitActors;
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Target = Overlap.GetActor();
			if (!Target || HitActors.Contains(Target)) continue;

			HitActors.Add(Target);

			FVector HitNormal = (Target->GetActorLocation() - PendingAOELocation).GetSafeNormal();
			if (HitNormal.IsNearlyZero())
			{
				HitNormal = FVector::UpVector;
			}

			ApplyHitToActor(Target, PendingDamage, PendingAOELocation, HitNormal);
		}
	}

	else if (HitQueryType == EHitQueryType::SpinSweep)
	{
		// 시간 만료시 종료
		if (SpinEndTime > 0.0 && World->GetTimeSeconds() >= SpinEndTime)
		{
			EndHitWindow();
			return;
		}

		const FVector CurrLoc = Owner->GetActorLocation();

		if (SpinPrevLoc.IsZero())
		{
			SpinPrevLoc = CurrLoc;
			return;
		}

		// 이동하면서도 누락 방지: Prev→Curr 구간을 Sphere로 Sweep
		TArray<FHitResult> Hits;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(SpinSweep), false, Owner);

		const bool bAny = World->SweepMultiByChannel(
			Hits,
			SpinPrevLoc,
			CurrLoc,
			FQuat::Identity,
			ECC_Pawn, 
			FCollisionShape::MakeSphere(SpinRadius),
			Params
		);

		const double Now = World->GetTimeSeconds();

		if (bDrawDebug)
		{
			DrawDebugLine(World, SpinPrevLoc, CurrLoc, bAny ? FColor::Red : FColor::Green, false, 0.05f, 0, 2.f);
			DrawDebugSphere(World, CurrLoc, SpinRadius, 16, FColor::Yellow, false, 0.05f, 0, 1.f);
		}

		if (bAny)
		{
			for (const FHitResult& H : Hits)
			{
				AActor* Target = H.GetActor();
				if (!Target || Target == Owner) continue;

				double* LastTime = SpinLastHitTime.Find(Target);
				if (LastTime && (Now - *LastTime) < SpinTickInterval)
					continue;

				SpinLastHitTime.Add(Target, Now);

				ApplyHitToActor(Target, SpinDamagePerTick, H.ImpactPoint, H.ImpactNormal);
			}
		}

		SpinPrevLoc = CurrLoc;
		return;
	}

}

void UCombatComponent::ApplyHitToActor(AActor* Target, float InDamage, const FVector& HitPoint, const FVector& HitNormal)
{
	if (!Target) return;

	float FinalDamage = InDamage;

	if (Target->GetClass()->ImplementsInterface(UDebuffBallTargetInterface::StaticClass()))
	{
		if (IDebuffBallTargetInterface::Execute_IsDebuffBallActive(Target))
		{
			FinalDamage *= IDebuffBallTargetInterface::Execute_GetDebuffBallIncomingDamageMultiplier(Target);
		}
	}

	if (Target->GetClass()->ImplementsInterface(UHitReactInterface::StaticClass()))
	{
		IHitReactInterface::Execute_OnHitReact(Target, FinalDamage, HitPoint, HitNormal);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("[FinalDamage] Raw=%.1f Final=%.1f"), InDamage, FinalDamage));
		}
	}
}

bool UCombatComponent::DoLineTraceMultiWithRange(TArray<FHitResult>& OutHits, float InRange) const
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	UWorld* World = Owner->GetWorld();
	if (!World) return false;

	ACharacter* Char = Cast<ACharacter>(Owner);
	ATP_Character* TPChar = Cast<ATP_Character>(Owner);

	FVector Start = Owner->GetActorLocation();
	if (Char && Char->GetMesh())
	{
		if (PendingTraceStartSocket != NAME_None && Char->GetMesh()->DoesSocketExist(PendingTraceStartSocket))
		{
			Start = Char->GetMesh()->GetSocketLocation(PendingTraceStartSocket);
		}
	}

	FVector Dir = Owner->GetActorForwardVector();

	if (TPChar && TPChar->HasValidLockOnTarget())
	{
		if (AActor* LockTarget = TPChar->GetLockOnTarget())
		{
			FVector TargetOrigin, TargetExtent;
			LockTarget->GetActorBounds(true, TargetOrigin, TargetExtent);

			const FVector ToTarget = (TargetOrigin - Start).GetSafeNormal();
			if (!ToTarget.IsNearlyZero())
			{
				Dir = ToTarget;
			}
		}
	}
	else
	{
		Dir = Owner->GetActorForwardVector().GetSafeNormal();
	}

	const FVector End = Start + Dir * InRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LocalAttackTraceMulti), false, Owner);

	const bool bHit = World->LineTraceMultiByChannel(
		OutHits,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	if (bDrawDebug)
	{
		DrawDebugLine(World, Start, End, bHit ? FColor::Red : FColor::Green, false, 1.5f, 0, 2.f);

		for (const FHitResult& Hit : OutHits)
		{
			DrawDebugPoint(World, Hit.ImpactPoint, 10.f, FColor::Red, false, 1.5f);
		}
	}

	return bHit;
}

bool UCombatComponent::DoSphereSweepMultiWithRange(TArray<FHitResult>& OutHits, float InRange, float InRadius, FName InStartSocket) const
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	UWorld* World = Owner->GetWorld();
	if (!World) return false;

	ACharacter* Char = Cast<ACharacter>(Owner);
	ATP_Character* TPChar = Cast<ATP_Character>(Owner);

	FVector Start = Owner->GetActorLocation();
	if (Char && Char->GetMesh())
	{
		if (InStartSocket != NAME_None && Char->GetMesh()->DoesSocketExist(InStartSocket))
		{
			Start = Char->GetMesh()->GetSocketLocation(InStartSocket);
		}
	}

	FVector Dir = Owner->GetActorForwardVector();

	if (TPChar && TPChar->HasValidLockOnTarget())
	{
		if (AActor* LockTarget = TPChar->GetLockOnTarget())
		{
			FVector TargetOrigin, TargetExtent;
			LockTarget->GetActorBounds(true, TargetOrigin, TargetExtent);

			const FVector ToTarget = (TargetOrigin - Start).GetSafeNormal();
			if (!ToTarget.IsNearlyZero())
			{
				Dir = ToTarget;
			}
		}
	}
	else
	{
		Dir = Owner->GetActorForwardVector().GetSafeNormal();
	}

	const FVector End = Start + Dir * InRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(BeamSweepMulti), false, Owner);

	const bool bHit = World->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(InRadius),
		Params
	);

	if (bDrawDebug)
	{
		DrawDebugLine(World, Start, End, bHit ? FColor::Blue : FColor::Cyan, false, 1.5f, 0, 2.f);
		DrawDebugSphere(World, Start, InRadius, 16, FColor::Blue, false, 1.5f, 0, 1.f);
		DrawDebugSphere(World, End, InRadius, 16, FColor::Blue, false, 1.5f, 0, 1.f);

		for (const FHitResult& Hit : OutHits)
		{
			DrawDebugPoint(World, Hit.ImpactPoint, 10.f, FColor::Blue, false, 1.5f);
		}
	}

	return bHit;
}

void UCombatComponent::ApplyHitResults(const TArray<FHitResult>& HitResults, float InDamage)
{
	TSet<AActor*> HitActors;
	AActor* Owner = GetOwner();

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;
		if (HitActor == Owner) continue;
		if (HitActors.Contains(HitActor)) continue;

		HitActors.Add(HitActor);

		FVector HitPoint = Hit.ImpactPoint;
		if (HitPoint.IsNearlyZero())
		{
			HitPoint = HitActor->GetActorLocation();
		}

		FVector HitNormal = Hit.ImpactNormal;
		if (HitNormal.IsNearlyZero())
		{
			HitNormal = -Owner->GetActorForwardVector();
		}

		ApplyHitToActor(HitActor, InDamage, HitPoint, HitNormal);
	}
}