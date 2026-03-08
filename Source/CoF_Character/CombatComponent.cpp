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
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::ConfigureTraceHit(float InDamage)
{
	HitQueryType = EHitQueryType::TraceForward;
	PendingDamage = InDamage;
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
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	ACharacter* Char = Cast<ACharacter>(Owner);
	APlayerController* PC = Char ? Cast<APlayerController>(Char->GetController()) : nullptr;
	if (!PC) return false;

	// 카메라 기반 트레이스: 화면 중앙 기준 조준 느낌을 얻기 좋음
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector Start = CamLoc;
	const FVector End = Start + CamRot.Vector() * TraceRange;

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
