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

	ExecuteHitOnce(); // Dash인 경우만 반복할 수 있도록
}


void UCombatComponent::ConfigureTraceHit(float InDamage, float InRange);
{
	HitQueryType = EHitQueryType::TraceForward;
	PendingDamage = InDamage;
	PendingRange = InRange;
}

void UCombatComponent::ConfigureDashHit(float InDamage, float InRange, float InDuration)
{
	HitQueryType = EHitQueryType::DashTrace;
	PendingDamage = InDamage;
	PendingRange = InRange;

	if (UWorld* W = GetWorld())
	{
		DashEndTime = W->GetTimeSeconds() + InDuration;
	}
	else
	{
		DashEndTime = 0.0;
	}

	DashHitActors.Reset();
}

void UCombatComponent::ConfigureAOEHit(float InDamage, float InRadius)
{
	HitQueryType = EHitQueryType::AOESphere;
	PendingDamage = InDamage;
	PendingRadius = InRadius;
}


void UCombatComponent::BeginHitWindow_OneShot()
{
	bHitWindowOpen = true;
	bHitAppliedThisSwing = false;

	ExecuteHitOnce();
}

void UCombatComponent::EndHitWindow()
{
	bHitWindowOpen = false;
}

void UCombatComponent::ExecuteHitOnce()
{
	if (!bHitWindowOpen) return;
	if (bHitAppliedThisSwing) return;

	AActor* Owner = GetOwner();
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	if (!World) return;

	if (HitQueryType == EHitQueryType::TraceForward)
	{
		FHitResult Hit;
		if (DoLineTrace(Hit) && Hit.GetActor())
		{
			ApplyHitToActor(Hit.GetActor(), PendingDamage, Hit.ImpactPoint, Hit.ImpactNormal);
		}
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
		// 기간 종료 처리
		if (DashEndTime > 0.0 && World->GetTimeSeconds() >= DashEndTime)
		{
			EndHitWindow();
			return;
		}

		FHitResult Hit;
		if (!DoLineTraceWithRange(Hit, PendingRange)) return;

		AActor* Target = Hit.GetActor();
		if (!Target || Target == Owner) return;

		if (DashHitActors.Contains(Target)) return; // 동일 대상 1회
		DashHitActors.Add(Target);

		ApplyHitToActor(Target, PendingDamage, Hit.ImpactPoint, Hit.ImpactNormal);

		if (bDrawDebug)
		{
			DrawDebugLine(World, Hit.TraceStart, Hit.TraceEnd, FColor::Yellow, false, 0.05f, 0, 1.5f);
		}
		return;
	}

	bHitAppliedThisSwing = true;
}

void UCombatComponent::ApplyHitToActor(AActor* Target, float InDamage, const FVector& HitPoint, const FVector& HitNormal)
{
	if (!Target) return;

	// 기존 콤보에서 쓰던 방식대로 HitReact 인터페이스로 데미지 전달
	if (Target->GetClass()->ImplementsInterface(UHitReactInterface::StaticClass()))
	{
		IHitReactInterface::Execute_OnHitReact(Target, InDamage, HitPoint, HitNormal);
	}
}


bool UCombatComponent::DoLineTrace(FHitResult& OutHit) const
{
	return DoLineTraceWithRange(OutHit, TraceRange);
}

bool UCombatComponent::DoLineTraceWithRange(FHitResult& OutHit, float InRange) const
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	ACharacter* Char = Cast<ACharacter>(Owner);
	APlayerController* PC = Char ? Cast<APlayerController>(Char->GetController()) : nullptr;
	if (!PC) return false;

	// 카메라 기반 트레이스: 화면 중앙 기준
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector Start = CamLoc;
	const FVector End = Start + CamRot.Vector() * InRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LocalAttackTrace), false, Owner);
	const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
		OutHit, Start, End, ECC_Visibility, Params);

	if (bDrawDebug)
	{
		DrawDebugLine(Owner->GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 1.5f, 0, 2.f);
		if (bHit)
			DrawDebugPoint(Owner->GetWorld(), OutHit.ImpactPoint, 12.f, FColor::Red, false, 1.5f);
	}

	return bHit;
}
