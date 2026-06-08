#include "ShijuArrowProjectile.h"

#include "ShijuQArea.h"
#include "ShijuBoss.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AShijuArrowProjectile::AShijuArrowProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    SetRootComponent(CollisionComp);
    CollisionComp->InitSphereRadius(8.f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    CollisionComp->SetNotifyRigidBodyCollision(true);
    CollisionComp->SetGenerateOverlapEvents(true);
    CollisionComp->OnComponentHit.AddDynamic(this, &AShijuArrowProjectile::OnProjectileHit);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 2000.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    Damage = 0.f;
    DamageCauserActor = nullptr;
    LinkedQArea = nullptr;
    bIsQProjectile = false;

    InitialLifeSpan = 10.f;
}

void AShijuArrowProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void AShijuArrowProjectile::InitProjectile(float InDamage, AActor* InDamageCauser, float InSpeed)
{
    Damage = InDamage;
    DamageCauserActor = InDamageCauser;

    if (ProjectileMovement)
    {
        ProjectileMovement->InitialSpeed = InSpeed;
        ProjectileMovement->MaxSpeed = InSpeed;
        ProjectileMovement->Velocity = GetActorForwardVector() * InSpeed;
    }

    if (CollisionComp && InDamageCauser)
    {
        CollisionComp->IgnoreActorWhenMoving(InDamageCauser, true);

        if (APawn* OwnerPawn = Cast<APawn>(InDamageCauser))
        {
            if (AController* OwnerController = OwnerPawn->GetController())
            {
                if (APawn* ControlledPawn = OwnerController->GetPawn())
                {
                    CollisionComp->IgnoreActorWhenMoving(ControlledPawn, true);
                }
            }
        }
    }
}

void AShijuArrowProjectile::InitQProjectile(AShijuQArea* InLinkedQArea)
{
    LinkedQArea = InLinkedQArea;
    bIsQProjectile = true;
}

void AShijuArrowProjectile::OnProjectileHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    if (OtherActor == GetOwner() || OtherActor == DamageCauserActor)
    {
        return;
    }

    if (bIsQProjectile && LinkedQArea)
    {
        LinkedQArea->ActivateBurnField(Hit.ImpactPoint);
        Destroy();
        return;
    }

    if (OtherActor && OtherActor != DamageCauserActor)
    {
        UGameplayStatics::ApplyPointDamage(
            OtherActor,
            Damage,
            GetActorForwardVector(),
            Hit,
            nullptr,
            DamageCauserActor,
            nullptr
        );

        if (APawn* HitPawn = Cast<APawn>(OtherActor))
        {
            if (HitPawn->IsPlayerControlled())
            {
                if (AShijuBoss* ShijuBoss = Cast<AShijuBoss>(DamageCauserActor))
                {
                    ShijuBoss->RegisterTimeMarkHit();
                }
            }
        }
    }

    Destroy();
}