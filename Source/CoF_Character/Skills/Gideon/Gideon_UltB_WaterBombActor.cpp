#include "Skills/Gideon/Gideon_UltB_WaterBombActor.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Skills/Gideon/Gideon_UltB_WaterBombDrop.h"

AGideon_UltB_WaterBombActor::AGideon_UltB_WaterBombActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 이 클래스는 CommonProjectile 목록에 넣기 위해 ACoF_CommonProjectile을 상속하지만,
	// 실제 판정은 projectile collision/movement가 아니라 Gideon_UltB_WaterBombDrop의 AOELocation으로 처리한다.
	if (Collision)
	{
		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Collision->SetGenerateOverlapEvents(false);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
		ProjectileMovement->ProjectileGravityScale = 0.f;
		ProjectileMovement->InitialSpeed = 0.f;
		ProjectileMovement->MaxSpeed = 0.f;
	}
}

void AGideon_UltB_WaterBombActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInitialized || bFinished)
	{
		return;
	}

	ElapsedTime += DeltaSeconds;

	const float Alpha = FallDuration <= KINDA_SMALL_NUMBER
		? 1.f
		: FMath::Clamp(ElapsedTime / FallDuration, 0.f, 1.f);

	const FVector NewLocation = FMath::Lerp(StartLocation, ImpactLocation, Alpha);
	SetActorLocation(NewLocation);

	if (Alpha >= 1.f)
	{
		FinishDrop();
	}
}

void AGideon_UltB_WaterBombActor::InitVisualBomb(
	UGideon_UltB_WaterBombDrop* InSkill,
	const FVector& InStartLocation,
	const FVector& InImpactLocation,
	float InFallDuration
)
{
	WaterBombSkill = InSkill;
	StartLocation = InStartLocation;
	ImpactLocation = InImpactLocation;

	FallDuration = FMath::Max(InFallDuration, 0.01f);
	ElapsedTime = 0.f;

	bInitialized = true;
	bFinished = false;

	SetActorLocation(StartLocation);

	BP_OnBombInitialized();
}

void AGideon_UltB_WaterBombActor::FinishDrop()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;

	BP_OnBombExploded();

	if (WaterBombSkill)
	{
		WaterBombSkill->ExplodeAtLocation(ImpactLocation);
	}

	Destroy();
}