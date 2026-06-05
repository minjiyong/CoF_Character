#include "Skills/Gideon/Gideon_UltB_WaterBombActor.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Skills/Gideon/Gideon_UltB_WaterBombDrop.h"

AGideon_UltB_WaterBombActor::AGideon_UltB_WaterBombActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 이 액터는 기존 Projectile 계열 BP 목록에 뜨기 위해 ACoF_CommonProjectile을 상속하지만,
	// 실제 판정은 CombatComponent의 AOELocation으로 처리하므로 Projectile 충돌/이동은 사용하지 않는다.
	if (Collision)
	{
		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Collision->SetSphereRadius(1.0f);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
		ProjectileMovement->Velocity = FVector::ZeroVector;
		ProjectileMovement->InitialSpeed = 0.0f;
		ProjectileMovement->MaxSpeed = 0.0f;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
	}
}

void AGideon_UltB_WaterBombActor::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}
}

void AGideon_UltB_WaterBombActor::InitVisualBomb(
	UGideon_UltB_WaterBombDrop* InOwningSkill,
	const FVector& InStartLocation,
	const FVector& InImpactLocation,
	float InFallDuration
)
{
	WaterBombSkill = InOwningSkill;
	StartLocation = InStartLocation;
	ImpactLocation = InImpactLocation;
	FallDuration = FMath::Max(InFallDuration, 0.01f);

	ElapsedTime = 0.0f;
	bInitialized = true;
	bExploded = false;

	SetActorLocation(StartLocation);

	BP_OnBombInitialized();
}

void AGideon_UltB_WaterBombActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInitialized || bExploded)
	{
		return;
	}

	ElapsedTime += DeltaSeconds;

	const float Alpha = FMath::Clamp(ElapsedTime / FallDuration, 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(StartLocation, ImpactLocation, Alpha);

	SetActorLocation(NewLocation);

	if (Alpha >= 1.0f)
	{
		FinishDrop();
	}
}

void AGideon_UltB_WaterBombActor::FinishDrop()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	BP_OnBombExploded();

	if (WaterBombSkill)
	{
		WaterBombSkill->ExplodeAtLocation(ImpactLocation);
	}

	Destroy();
}