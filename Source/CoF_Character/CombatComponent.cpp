#include "CombatComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "HitReactInterface.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::TryAttack_Local()
{
	FHitResult Hit;
	const bool bHit = DoLineTrace(Hit);

	if (bHit && Hit.GetActor())
	{
		AActor* HitActor = Hit.GetActor();

		if (HitActor->GetClass()->ImplementsInterface(UHitReactInterface::StaticClass()))
		{
			IHitReactInterface::Execute_OnHitReact(HitActor, Damage, Hit.ImpactPoint, Hit.ImpactNormal);
		}

		UE_LOG(LogTemp, Warning, TEXT("[Attack] Hit=%s Loc=%s Normal=%s"),
			*Hit.GetActor()->GetName(),
			*Hit.ImpactPoint.ToString(),
			*Hit.ImpactNormal.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack] Miss"));
	}
}

void UCombatComponent::BeginHitWindow_OneShot()
{
	bHitWindowOpen = true;
	bHitAppliedThisSwing = false;

	// "한 번만 맞게" -> HitStart에서 즉시 1회 실행
	if (!bHitAppliedThisSwing)
	{
		TryAttack_Local();
		bHitAppliedThisSwing = true;
	}
}

void UCombatComponent::EndHitWindow()
{
	bHitWindowOpen = false;
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
